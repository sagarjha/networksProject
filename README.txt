CS 378 : Computer Networks

Project Name : FileMesh

Member:	Ayush Kanodia - 110050049
	Sagar Jha     - 110100024
	Raghav Gupta  - 110100092

List of Relevant Files :	user.c		-- Includes code for the program that runs at the client end
     		       		server.c	-- Includes code for the program that runs at the server end
				config.h	-- Configures the server by reading the file "FileMesh.cfg"
				Makefile	-- Compiles automatically the source file into appropriate binaries
				startServer.sh	-- starts the servers that belong to the machine on which it is run (the optional part)
				FileMesh.cfg	-- contains the IP addresses, port numbers and directories (to store the files in) for the server
				Doxyfile	-- contains the code to document the project
								
				-- Files that calculate md5 sum (minor adjustments to the library available on internet)
				md5c.c
				mddriver.c
				global.h
				md5.h

Compilation Instructions :	"make server"		-- to compile the server program, creates an executable "server"
	    		 	"make user"		-- to compile the user program, creates an executable "user"
				"make" or "make all"	-- to compile both the server and user program, creates executables "server" and "user"
				"make doc"		-- to create the documentation, creates an html directory and latex directory
				"make clean"		-- to clean up all the executables, html directory and latex directory

Configuration Files :		No such files from our end

Running Instructions :		"./server <server-id>"			-- start the server with the id <server-id>
		     		"./user 0 <filename for send>		-- 0 is for send, filename is just the name (not with directory structure)
					<directory ending with a '/'>	-- location of the file, relative to the current directory
					<remoteIP> 	       	 	-- IP of the server to contact e.g. 10.105.12.13
					<remote Port Number> 		-- port number of the server to contact
					<selfIP>"    			-- IP address of the machine on which this command is run with which the server is to contacted
				"./user 1 <md5sum for retrieve>		-- 1 is for send, md5 sum is in 32 hex characters with lower case for the 6 english characters
					<directory ending with a '/'>	-- location to store the file in, relative to the current directory
					<remoteIP> 	       	 	-- IP of the server to contact e.g. 10.105.12.13
					<remote Port Number> 		-- port number of the server to contact
					<selfIP>"    			-- IP address of the machine on which this command is run with which the server is to contacted
				./startServer.sh			-- to starts the servers that belong to the machine on which it is run (the optional part)
				"firefox html/index.html"		-- to open the documentation html
				"evince latex/refman.pdf"		-- to open the reference manual, instead of evince use your favourite pdf viewer

				    	 
Notes :				Before running the servers or executing startServers.sh (script file), it should be ensured that the directory to store the files is present in the directory inside which the server is running
      				For retrieving, the md5 sum given must be 32 hex characters with lower case for the 6 english characters
				When a file is stored, it replaces any file which corresponded to the same md5 sum as of this file. As a corollary, if the user tries to store the same file (with the same contents), it replaces the older file
