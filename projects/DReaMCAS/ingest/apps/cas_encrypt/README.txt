=============
  encryptTs
=============

This implementation takes in a MPEG-2 TS file and outputs an encrypted file using either a provided or default stream key.  The access keys can be output to an EMM file.

encryptTs [-d ] [-c <Ecm file>] [-m <Emm file>] [-t <ECM Inserted TS file>] <mpeg2 ts file> <output ts file>
                  -d    Use Debug Encryption Keys
                  -c    file ECMs are written to
                  -m    file to write EMM into
                  -t    temporary TS file with ECM TS inserted (no scrambling on Elementary streams)


Example

# ./encryptTs -d -m javacar.emm javacar.mpg javacar.en.mpg

Encrypt an MPEG-2 TS file "javacar.mpg" using the default stream key and output the encrypted MPEG-2 TS file "javacar.en.mpg" and the EMM file "javacar.emm".



==============
  emmEncrypt
==============

The purpose is to encrypt a given EMM file.  In the reference architecture, this encrypted EMM file is stored in the database for the License Server to provide this for clients. 

emmEncrypt <EMM file> <pub.key file> <encrypted EMM file>

	<Input: EMM in TS file name>  
        <Input: Client RSA Public Key file name> 
        <Output: Encrypted file name>

Example 

# ./emmEncrypt javacar.emm pub.key javacar.en.emm

Encrypt an EMM file "javacar.emm" using the public key file "pub.key" and output the encrypted EMM to the file "javacar.en.emm".
