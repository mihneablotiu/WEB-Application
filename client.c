#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <stdbool.h>
#include "helpers.h"
#include "requests.h"
#include "buffer.h"
#include "parson.c"

#define HOST "34.241.4.235"
#define PAYLOAD_TYPE "application/json"

/* The function that interprets the register command when the user wants to
create a new account in the server.
   @sockfd -> the socket file descriptor of the connexion
*/
void register_command_interpretation(int sockfd) {
    char *message, *response;

    char **form_data = calloc(1, sizeof(char *));
    form_data[0] = calloc(LINELEN, sizeof(char));
        
    char username[LINELEN];
    char password[LINELEN];

    // We read the username and the password of the new account
    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    /* We create the JSON_Object with the username and the password and then
    we serialize it */
    JSON_Value *new_value = json_value_init_object();
    JSON_Object *new_object = json_value_get_object(new_value);
    char *serialized_string = NULL;

    json_object_set_string(new_object, "username", username);
    json_object_set_string(new_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(new_value);
    memcpy(form_data[0], serialized_string, strlen(serialized_string));

    // We compute the POST request and then send it to the server to the specific access route
    message = compute_post_request(HOST, "/api/v1/tema/auth/register", PAYLOAD_TYPE, form_data, 1, NULL, 0, 0);
    send_to_server(sockfd, message);

    /* We get the response from the server and we parse the answer. Also we inform the user
    with a specific message depending on the answer we get from the server */
    response = receive_from_server(sockfd);

    if (strstr(response, "error") != NULL) {
        if (strstr(response, ("requests")) == NULL) {
            printf("%s\n", "Username already in use!");
        } else {
            printf("%s\n", "Too many requests, please try again later!");
        }
        
    } else {
        printf("%s\n", "Client succesfully registered!");
    }
    
    free(message);
    free(response);
    free(form_data[0]);
    free(form_data);

    json_free_serialized_string(serialized_string);
    json_value_free(new_value);
}

/* The function that interprets the login command when the user wants to
join an already existing account on the server.
   @sockfd -> the socket file descriptor of the connexion
*/
char *login_command_interpretation(int sockfd) {
    char *message, *response;

    char **form_data = calloc(1, sizeof(char *));
    form_data[0] = calloc(LINELEN, sizeof(char));
        
    char username[LINELEN];
    char password[LINELEN];

    // We read the username and the password for the already existing account
    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    /* We create the JSON_Object with the username and the password we just read
    and we serialize it */
    JSON_Value *new_value = json_value_init_object();
    JSON_Object *new_object = json_value_get_object(new_value);
    char *serialized_string = NULL;

    json_object_set_string(new_object, "username", username);
    json_object_set_string(new_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(new_value);
    memcpy(form_data[0], serialized_string, strlen(serialized_string));

    // We compute the POST request to the specific access route and we send it to the server
    message = compute_post_request(HOST, "/api/v1/tema/auth/login", PAYLOAD_TYPE, form_data, 1, NULL, 0, 0);
    send_to_server(sockfd, message);

    /* We get the response from the server, we parse the response and we inform the client about the
    error / result we received. */
    response = receive_from_server(sockfd);

    char *cookie = NULL;
    if (strstr(response, "error") != NULL) {
        if (strstr(response, "account") != NULL) {
            printf("%s\n", "This client username does not exist!");
        } else {
            printf("%s\n", "The combination between username and password is not correct!");
        }
    } else {
        printf("%s\n", "Welcome!");
        
        /* If the answer is an afirmative one we have to extract the session cookie from the
        answer and return it to the client because we need as long as we stay connected */
        cookie = strdup(strtok(response, "\r\n"));
        while (cookie != NULL) {
            if (strncmp(cookie, "Set-Cookie:", 11) == 0) {
                break;
            }

            free(cookie);
            cookie = strdup(strtok(NULL, "\r\n"));
        }
    }
    
    free(message);
    free(response);
    free(form_data[0]);
    free(form_data);

    json_free_serialized_string(serialized_string);
    json_value_free(new_value);

    return cookie;
}

/* The function that interprets the enter_library command when the user wants
to join the library after he logged in his account.
    @sockfd -> the socket file descriptor of the connexion
    @cookie -> the session cookie we have to include in the get message because
               we have to prove the fact that we are already logged in
*/
char *enter_library_command_interpretation(int sockfd, char *cookie) {
    char *message, *response;

    // We include the session cookie in our cookies matrix
    char **cookies = calloc(1, sizeof(char *));
    cookies[0] = calloc(LINELEN, sizeof(char));
    memcpy(cookies[0], cookie, strlen(cookie));

    // We compute the GET message to the specific address and we send it to the server
    message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, cookies, 1);
    send_to_server(sockfd, message);

    /* We wait for the server's response, we parse his answer and inform the client
    about the status of the command. */
    response = receive_from_server(sockfd);
    printf("%s\n", "Client succesfully registered into the library!");

    /* We have to extract the JWT token from the answer which is going to be our
    proof of having access to the library */
    char *tokenJWT = NULL;
    tokenJWT = strdup(strtok(response, "{"));
    free(tokenJWT);
    tokenJWT = strdup(strtok(NULL, "{"));

    char *finalToken = (char *)calloc(BUFLEN, sizeof(char));
    finalToken[0] = '{';
    strncat(finalToken, tokenJWT, strlen(tokenJWT));

    free(tokenJWT);
    free(message);
    free(response);
    free(cookies[0]);
    free(cookies);

    return finalToken;
}

/* The function that interprets the logout command when the user wants to
exit his account (if he is already logged in)
    @sockfd -> the socket file descriptor of the connexion
    @cookie -> the session cookie we have to include in the get message because
               we have to prove the fact that we are already logged in in order
               to be able to log out.
*/
void logout_command_interpretation(int sockfd, char *cookie) {
    char *message, *response;

    char **form_data = calloc(1, sizeof(char *));
    form_data[0] = calloc(LINELEN, sizeof(char));
    memcpy(form_data[0], "enter_library", strlen("enter_library"));

    // We include the cookie to our cookies matrix
    char **cookies = calloc(1, sizeof(char *));
    cookies[0] = calloc(LINELEN, sizeof(char));
    memcpy(cookies[0], cookie, strlen(cookie));

    // We compute the GET message and we send it to the server
    message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, cookies, 1);
    send_to_server(sockfd, message);

    // We receive the server's response and we inform the client about the status of the command
    response = receive_from_server(sockfd);
    printf("%s\n", "You have been logged out!");

    free(message);
    free(response);
    free(form_data[0]);
    free(form_data);
    free(cookies[0]);
    free(cookies);
}

/* The function that interprets the get_books command when the user wants to
get information about all the books that exist at a specific moment in the
library
    @sockfd -> the socket file descriptor of the connexion
    @tokenJWT -> the proof of having access into the library
*/
void get_books_command_interpretation(int sockfd, char *tokenJWT) {
    char *message, *response;
    char line[LINELEN];

    // We parse the token in order to get just the value from the JSON_Object
    JSON_Value *new_value = json_parse_string(tokenJWT);
    JSON_Object *new_object = json_value_get_object(new_value);
    const char *finalToken = json_object_get_string(new_object, "token");

    // We compute the get request and then we add the token to the end of it
    message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, NULL, 0);
    memset(message + strlen(message) - 2, 0, 2);
    sprintf(line, "Authorization: Bearer %s", finalToken);
    compute_message(message, line);
    compute_message(message, "");

    // We send the message to the server and then we are waiting for a response
    // from the server
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // We extract the JSON_Array of books from the server's answer and we parse it
    // in order to offer a pretty looking answer to the client
    char *books = strstr(response, "[{");
    if (books != NULL) {
        JSON_Value *books_value = json_parse_string(books);
        JSON_Array *books_array = json_value_get_array(books_value);

        for (size_t i = 0; i < books_array->count; i++) {
            JSON_Object *book_object = json_array_get_object(books_array, i);
            printf("Id: %d\n", (int) json_object_get_number(book_object, "id"));
            printf("Title: %s\n\n", json_object_get_string(book_object, "title"));
        }

        json_value_free(books_value);
    } else {
        printf("[]\n");
    }

    json_value_free(new_value);
    free(message);
    free(response);
}

/* The function that interprets the add_book command when the user wants to
add a book to the library
    @sockfd -> the socket file descriptor of the connexion
    @tokenJWT -> the proof of having access into the library
*/
void add_book_command_interpretation(int sockfd, char *tokenJWT) {
    char *message;

    char **form_data = calloc(2, sizeof(char *));
    form_data[0] = calloc(LINELEN, sizeof(char));
    form_data[1] = calloc(LINELEN, sizeof(char));

    // We parse the token in order to get just the value from the JSON_Object
    JSON_Value *token_value = json_parse_string(tokenJWT);
    JSON_Object *token_object = json_value_get_object(token_value);
    const char *finalToken = json_object_get_string(token_object, "token");
    sprintf(form_data[0], "Authorization: Bearer %s", finalToken);
        
    /* We read all the information of the new book from the keyboard such as
    the title, the author, the genre, the publisher and the page_count */
    char title[LINELEN], author[LINELEN], genre[LINELEN], publisher[LINELEN];
    char page_count_string[LINELEN];
    int page_count;

    printf("title=");
    fgets(title, LINELEN, stdin);
    title[strlen(title) - 1] = '\0';
    printf("author=");
    fgets(author, LINELEN, stdin);
    author[strlen(author) - 1] = '\0';
    printf("genre=");
    fgets(genre, LINELEN, stdin);
    genre[strlen(genre) - 1] = '\0';
    printf("publisher=");
    fgets(publisher, LINELEN, stdin);
    publisher[strlen(publisher) - 1] = '\0';
    printf("page_count=");
    fgets(page_count_string, LINELEN, stdin);

    // We take care of the page_count in order to be a number
    if (atoi(page_count_string) == 0) {
        printf("%s\n", "Page count is not a number, please enter valid information");
        free(form_data[0]);
        free(form_data[1]);
        free(form_data);
        json_value_free(token_value);
        return;
    } else {
        page_count = atoi(page_count_string);
    }

    /* We create the new JSON_Object with all the information we just read from
    the keyboard and then we serialize it */
    JSON_Value *new_value = json_value_init_object();
    JSON_Object *new_object = json_value_get_object(new_value);
    char *serialized_string = NULL;

    json_object_set_string(new_object, "title", title);
    json_object_set_string(new_object, "author", author);
    json_object_set_string(new_object, "genre", genre);
    json_object_set_number(new_object, "page_count", page_count);
    json_object_set_string(new_object, "publisher", publisher);
    serialized_string = json_serialize_to_string_pretty(new_value);
    memcpy(form_data[1], serialized_string, strlen(serialized_string));

    /* We compute the message with the JWT Token in the body, we move it as a header and then we add
    the book information as a payload of the message */
    message = compute_post_request(HOST, "/api/v1/tema/library/books", PAYLOAD_TYPE, form_data, 1, NULL, 0, strlen(form_data[1]));
    memset(message + strlen(message) - strlen(form_data[0]) - 2, 0, 2 + strlen(form_data[0]));
    compute_message(message, form_data[0]);
    compute_message(message, "");
    compute_message(message, form_data[1]);
    memset(message + strlen(message) - 2, 0, 2);

    // We send the message to the server and then we inform the user about the response
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    printf("%s\n", "Your book has been added to the library");

    free(message);
    free(response);
    free(form_data[0]);
    free(form_data[1]);
    free(form_data);
    json_free_serialized_string(serialized_string);
    json_value_free(new_value);
    json_value_free(token_value);
}

/* The function that interprets the get_book command when the user wants to
get the information about a specific book that exists in the library
    @sockfd -> the socket file descriptor of the connexion
    @tokenJWT -> the proof of having access into the library
*/
void get_book_command_interpretation(int sockfd, char *tokenJWT) {
    char *message, *response;
    char line[LINELEN], id[LINELEN];

    // We read the id of the book that wants to be checked
    printf("id=");
    fgets(id, LINELEN, stdin);

    /* We take care if the book id is a number or not and we return
    if it is not a number */
    if (atoi(id) == 0) {
        printf("%s\n", "The Id you have entered is not a number. Returning!");
        return;
    }

    // We parse the token in order to get just the value from the JSON_Object
    JSON_Value *new_value = json_parse_string(tokenJWT);
    JSON_Object *new_object = json_value_get_object(new_value);
    const char *finalToken = json_object_get_string(new_object, "token");

    // We create the specific path including the id we have just read
    char path[] = "/api/v1/tema/library/books/";
    strcat(path, id);
    path[strlen(path) - 1] = ' ';

    /* We create the message and then we add the JWT token to prove the access
    to the library */
    message = compute_get_request(HOST, path, NULL, NULL, 0);
    memset(message + strlen(message) - 2, 0, 2);
    sprintf(line, "Authorization: Bearer %s", finalToken);
    compute_message(message, line);
    compute_message(message, "");

    // We send the message to the server and then we get the response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    
    /* We parse the response and we check if it exists a book with the 
    given ID and if it does, then we parse the JSON from the response in order
    to give a pretty looking answer to the client*/
    if (strstr(response, "No book") != NULL) {
        printf("%s\n", "There is no book with this ID! Returning");
    } else {
        char *book = strstr(response, "[{");
        JSON_Value *book_value = json_parse_string(book);
        JSON_Array *book_array = json_value_get_array(book_value);
        
        for (size_t i = 0; i < book_array->count; i++) {
            JSON_Object *book_object = json_array_get_object(book_array, i);
            printf("Title: %s\n", json_object_get_string(book_object, "title"));
            printf("Author: %s\n", json_object_get_string(book_object, "author"));
            printf("Publisher: %s\n", json_object_get_string(book_object, "publisher"));
            printf("Genre: %s\n", json_object_get_string(book_object, "genre"));
            printf("Page Count: %d\n", (int) json_object_get_number(book_object, "page_count"));
        }
        
        json_value_free(book_value);
    }

    json_value_free(new_value);
    free(message);
    free(response);
}

/* The function that interprets the delete_book command when the user wants
to delete an existing book in the library
    @sockfd -> the socket file descriptor of the connexion
    @tokenJWT -> the proof of having access into the library
*/
void delete_command_interpretation(int sockfd, char *tokenJWT) {
    char *message, *response;
    char line[LINELEN], id[LINELEN];

    // We read the id of the book that wants to be checked
    printf("id=");
    fgets(id, LINELEN, stdin);

    /* We take care if the book id is a number or not and we return
    if it is not a number */
    if (atoi(id) == 0) {
        printf("%s\n", "The Id you have entered is not a number. Returning!");
        return;
    }

    // We parse the token in order to get just the value from the JSON_Object
    JSON_Value *new_value = json_parse_string(tokenJWT);
    JSON_Object *new_object = json_value_get_object(new_value);
    const char *finalToken = json_object_get_string(new_object, "token");

    // We create the specific path including the id we have just read
    char path[] = "/api/v1/tema/library/books/";
    strcat(path, id);
    path[strlen(path) - 1] = ' ';

    /* We create the message and then we add the JWT token to prove the access
    to the library */
    message = compute_delete_request(HOST, path, NULL, NULL, 0);
    memset(message + strlen(message) - 2, 0, 2);
    sprintf(line, "Authorization: Bearer %s", finalToken);
    compute_message(message, line);
    compute_message(message, "");

    // We send the message to the server and then we get the response
    send_to_server(sockfd, message);

    /* We inform the user about the answer of the server meaning if the book has been
    deleted or if the given id does not exist */
    response = receive_from_server(sockfd);
    if (strstr(response, "No book") != NULL) {
        printf("%s\n", "There is no book with this ID! Returning");
    } else {
        printf("%s\n", "Your book has been successfully deleted!");
    }
    
    json_value_free(new_value);
    free(message);
    free(response);
}

int main(void) {
    int sockfd, cookieFreed = 1, tokenJWTFreed= 1, loggedIn = 0;
    char *cookie = NULL, *tokenJWT = NULL;
    char command[BUFLEN];
    buffer reader = buffer_init();

    /* The main part of the program where we open a connexion with the server for each
    command and we do a specific action depending on what does the user enter from the
    keyboard.
    
    We also take care about some obvious errors and we don't try to overload the server
    in the situations we know for sure that is going to return an error (e.g. trying to
    access the library without being logged in, etc.). */
    while (true) {
        fgets(command, BUFLEN - 1, stdin);
        buffer_add(&reader, command, strlen(command) + 1);
        
        sockfd = open_connection(HOST, 8080, AF_INET, SOCK_STREAM, 0);

        if (buffer_find(&reader, "register", strlen("register")) != -1) {
            if (loggedIn == 0) {
                register_command_interpretation(sockfd);
            } else {
                printf("%s\n", "You can't register a new account while you are logged in!");
            }
        } else if (buffer_find(&reader, "login", strlen("login")) != -1) {
            if (loggedIn == 0) {
                cookie = login_command_interpretation(sockfd);
                if (cookie != NULL) {
                    char *cookieValue = strdup(strtok(cookie, " "));
                    free(cookieValue);
                    cookieValue = strdup(strtok(NULL, " "));
                    free(cookie);
                    cookie = strdup(cookieValue);
                    free(cookieValue);
                    cookieFreed = 0;
                    loggedIn = 1;
                }
            } else {
                printf("%s\n", "You are already logged in!");
            }
        } else if (buffer_find(&reader, "enter_library", strlen("enter_library")) != -1) {
            if (cookie != NULL) {
                if (tokenJWTFreed == 0) {
                    free(tokenJWT);
                    tokenJWT = enter_library_command_interpretation(sockfd, cookie);
                } else {
                    tokenJWT = enter_library_command_interpretation(sockfd, cookie);
                    tokenJWTFreed = 0;
                }
            } else {
                printf("%s\n", "You are not logged in!");
            }
        } else if (buffer_find(&reader, "get_books", strlen("get_books")) != -1) {
            if (loggedIn != 0) {
                if (tokenJWT != NULL) {
                    get_books_command_interpretation(sockfd, tokenJWT);
                } else {
                    printf("%s\n", "You don't have access to the library!");
                }
            } else {
                printf("%s\n", "You are not logged in!");
            }
        } else if (buffer_find(&reader, "get_book", strlen("get_book")) != -1) {
            if (loggedIn != 0) {
                if (tokenJWT != NULL) {
                    get_book_command_interpretation(sockfd, tokenJWT);
                } else {
                    printf("%s\n", "You don't have access to the library!");
                }
            } else {
                printf("%s\n", "You are not logged in!");
            }
        } else if (buffer_find(&reader, "add_book", strlen("add_book")) != -1) {
            if (loggedIn != 0) {
                if (tokenJWT != NULL) {
                    add_book_command_interpretation(sockfd, tokenJWT);
                } else {
                    printf("%s\n", "You don't have access to the library!");
                }
            } else {
                printf("%s\n", "You are not logged in!");
            }
        } else if (buffer_find(&reader, "delete_book", strlen("delete_book")) != -1) {
            if (loggedIn != 0) {
                if (tokenJWT != NULL) {
                    delete_command_interpretation(sockfd, tokenJWT);
                } else {
                    printf("%s\n", "You don't have access to the library!");
                }
            } else {
                printf("%s\n", "You are not logged in!");
            }
        } else if (buffer_find(&reader, "logout", strlen("logout")) != -1) {
            if (loggedIn == 1) {
                logout_command_interpretation(sockfd, cookie);
                loggedIn = 0;
                if (cookieFreed == 0) {
                    free(cookie);
                    cookie = NULL;
                    cookieFreed = 1;
                }

                if (tokenJWTFreed == 0) {
                    free(tokenJWT);
                    tokenJWT = NULL;
                    tokenJWTFreed = 1;
                }
            } else {
                printf("%s\n", "You are not logged in!");
            }
        } else if (buffer_find(&reader, "exit", strlen("exit")) != -1) {
            buffer_destroy(&reader);

            if (cookieFreed == 1 && tokenJWTFreed == 1) {
                close(sockfd);
                return 0;
            } else if (cookieFreed == 1 && tokenJWTFreed == 0) {
                free(tokenJWT);
                close(sockfd);
                return 0;
            } else if (cookieFreed == 0 && tokenJWTFreed == 1) {
                free(cookie);
                close(sockfd);
                return 0;
            } else {
                break;
            }
        } 

        buffer_destroy(&reader);
    }

    free(tokenJWT);
    free(cookie);
    close(sockfd);
    return 0;
}
