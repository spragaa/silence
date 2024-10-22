## routes:
"/upload/:filename";
"/download/:filename";
"/delete/:filename";
"/list";


what to do in case of collisions on file server side?
I think we can simply craete new hash -> should add new logic to handle it

Current state:
once file server client receives the file, it can upload it to file server

1. ask user for the file name in his local env
2. async send default message with filename if provided
3. run another async function, that will send requests with chunks of data (filename, number of chunk, is last, chunk data)
4. server receives response to send message -> write filename into metadata, continue normal operations
5. server receives response with a chunk of data -> file_server_client sends it directly to file_server (file is stored in ram only)
6. step 5 repeats n times, where n - number of file chunks
7. once done, file is stored on the file_server, send response to the client that file is succesfully processed 
8. sending file to the receiver should be uno reverse, (I'm not sure if it make sense to directly send file from char server to receiver tho)
  server downloads chunks of file from file_server, and sends these chunks to the receiver client, receiver client on the other hand packs the file into 
  one thing and store locally
*** lets first implement the logic to store file on file_server and then download it from there
    then when we will be able to handle online and offline users, then I think I could think 
    about adding sender -> server -> receiver and not
    not sender -> server -> file_server -> server -> receiver

## file sending workflow
1. client chooses the media file 
2. we have a connection beetwen server and client -> we can send the file fully as a binary stream
3. on server side we generate the hash name for the file and saves it message metadata
   we have somekind of file packer worker, that packs the binary data into packages 
   we also have a file server client that takes these packages and sends post requests to the file server (name)
4. file server 'merges' files into one (i think)
5. in case of the name collision, file server responses with 'name collision detected' -> server regenerates the hash -> retry step 3

## folder structure 
use hash funvtion to determine file location, I think we can use first 512 bytes as hash input 
  output: string, for example abcdekjsdf34 (but unified lenght)
  -> file location is ../ab/cd/ekjsdf34.txt -> (62*62)^2 folders 

