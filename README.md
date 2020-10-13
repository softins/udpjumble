# udpjumble

This test program is a UDP proxy to be run in front of a Jamulus server,
to artifically introduce some audio packet misordering from the client
to the server.

It is used for testing the audio sequencing modifications to Jamulus at
https://github.com/softins/jamulus/tree/sequence-audio

See the temporary repo at https://github.com/softins/jamulus-seq-test
for the Mac and PC clients with this test feature.

`udpjumble` should be run on the same host as the Jamulus server, and currently,
it only supports a single client at a time, so it is more applicable to a
local test server than a public one.

```
usage: ./udpjumble [options]
options:
   -p port     Port to listen on (default 22124)
   -s port     Port for localhost server (default 22123)
   -?          Help (this message)
```
