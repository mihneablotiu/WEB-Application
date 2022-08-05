Blotiu Mihnea-Andrei - 323CA
Protocoale de comunicatie - Tema 3 - Web client


The main ideea of this homework is to implement a web client that can
communicate with an REST API, for managing a virtual library by adding books,
deleting books, registering new accounts, viewing the already existing books,
etc.

The project was done accordingly to the indications and was structured as follows:

	- buffer.c/buffer.h the functions and the headers of the functions that
	were used for reading, checking commands read from the keyboard that the
	client was giving;
	- client.c the heart of the application. It has the main function, reads
	the commands from the user and calls the specific function depending on
	the command;
	- helpers.c/helpers.h the functions and the headers of the functions that
	were used to establish the connexion with the server, send and receive
	messages from it;
	- parson.c/parson.h JSON Library used to create or parse JSON Objects
	in order to integrate them into the messages;
	- requests.c/requests.h the functions and the headers of the functions that
	were used to compose a GET/POST/DELETE request depending on what type of
	a message we had to send to the server;
	- Makefile;
	- README - the documenation of the project.
	
In this project I implemented the following aspects:
	
	- Register command for new accounts - 1p;
	- Login command for existing accounts - 1p;
	- Enter_library command in order to access a library after you are logged
	in - 1p;
	- Get_books command in order to see all the books that exist at a specific
	moment into the library - 2p;
	- Get_book command in order to see more detailed information about a
	specific book that exists into the library based on it's Id - 1p;
	- Add_book command in order to add a book to the library with all it's
	information such as: title, author, genre, publisher, page_count - 2p;
	- Delete_book command in order to delete an existing book in the library
	based on it's id - 1p;
	- Logout command for exiting an account you joined - 0.5p;
	- Exit command that stops the interaction between the client and the server
	and exits the program - 0.5p;
	- Valgrind clean - No memory leaks.
	
	Total: 10p.
	
The whole flow of the project is described by the client.c file as follows:

	- First of all we initialize our buffers and structures that we are going
	to work with. Then, our program is a while true loop waiting for commands
	and exiting just when the "exit" command comes around;
	- That being said, in each of the loops, we read the new command from the
	keyboard, we establish a new HTTP connection with the server and we do a
	different action depending on the command;
	- In this step we firstly check for obvious errors that can appear before
	sending a message to the server such as: you cannot register a new account
	if you are logged in, you cannot log in to a different account if you are
	already logged in without logging out, you cannot enter the library if you
	are not logged in, you cannot add/view/delete books if you don't have access
	to the library and also you cannot logout if you are not logged in;
	- After all these checks have been done, if everything is okey, we do a specific
	action as follows:
	
	* register:
		- We read the username and the password for the new account that we want
		to register;
		- We create the JSON object with these information, we compute a post
		request and we send it to the server;
		- We check the server's answer as there can already be an account with
		the same username. If not, then the register was succesfully done.
		
	* login:
		- Identical with the register with the difference that the answer from
		the server can refer to: an unexisting username if you try to login 
		to a username that was not registered before or to a bad combination
		between the username and the password;
		- If the command finished okey, the server is going to give back a
		sesion cookie that we need as a proof that we are already logged
		in;
		
	* enter_library:
		- We send a simple GET message because we want to get access to the
		library, but we include the session cookie into the message that we 
		got from the login command;
		- The server is going to send back a JWT token as a proof for our
		access to the library (different from the cookie that is a proof for
		our login);
		
	*logout:
		- We send a simple GET message because we want to logout from the
		server, but we include the session cookie into the message that we 
		got from the login command because we firstly have to prove that
		we are logged in, in order to be able to log out;
	
	*get_books:
		- We send a simple GET message because we want to get all the
		information about all the books;
		- We include the JWT Token into the message as a proof for our
		access to the library;
		- The response is going to be a JSON Array containing all the books
		from a specific moment into the library. We parse the array and print
		all the information in a pretty way for the client;
		
	* add_book:
		- We read all the information about the new book that we want to
		introduce to the library and we also take care about the data
		validation (for example we cannot accept a book that has a string
		as a page number);
		- We create a new JSON Object with all the information we just read;
		- We create a POST request because we want to add information on the
		server. We add the JWT Token as a header and then the JSON Obeject
		that was just created as a payload and we send the message;
		- We will recieve a confirmation that our book has been added;
		
	*get_book:
		- Identical with the get_books command with the difference that now
		we read an Id which is the id of the book we want to get the
		information for;
		- Then we send the get request with the JWT Token of course;
		- The output is going to be slightly different with much more detailed
		information about the book. We parse the answer and we print it to
		the user.
		
	*delete_book:
		- Identical with the get_book command with the only difference that we
		are not sending a GET request, but a DELETE request depending on the ID
		we read from the keyboard.
		
For the JSON creating/parsing parts of the project I chose to use the parson
library as mentioned below in the references because I found it as a perfect match
for the C language in which this program is being written and it also made my work
a lot easier because I didn't have to write the JSON parsing part.


References:

	1. HTTP Protocol laboratory - https://ocw.cs.pub.ro/courses/pc/laboratoare/10;
	2. Parson JSON library - https://github.com/kgabis/parson;
	

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
