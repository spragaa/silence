## routes:
"/upload/:filename";
"/download/:filename";
"/delete/:filename";
"/list";


what to do in case of collisions on file server side?
I think we can simply craete new hash -> should add new logic to handle it

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

