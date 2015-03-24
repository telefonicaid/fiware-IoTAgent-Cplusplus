FORMAT: 1A

# IoT Agent Thinking Things POST Documentation
The IoT Agent for Thinking Things is listening to HTTP POST requests. 

**Allowed HTTPs requests:**

- POST: devices send their measures and status. 

**Typical Server Responses:**

- 200 OK: The request was succesful. Thinking Things' modules information will be published on ContextBroker.
- 400 Bad Request: whenever the request cannot be interpreted by the agent. 
- 404 Not Found: when the device id was not properly provisioned.
- 500 Internal Server Error: Generic error when server has a malfunction.

Responses related with authentication and authorization depends on this feature is configured and a Keystone OpenStack sytem is present.

When an error is returned, a representation is returned as:

```
{
  "reason": "contains why an error is returned",
  "details": "contains specific information about the error, if possible"
}
```


### Send measures [POST]
 
Body has the keyword "cadena=" and the rest of the frame must be URL encoded:

+ Request 

    + Body

            cadena=%2389340753790000%2C%230%2CGM%2Ctemp%2C25%2C20%24wakeUp


+ Response 200

    + Body

            #0,GM,temp,20$wakeUp

It can happen that one defective module sends out an incorrect frame, the response will be 200 OK and ignoring the offending module.
Example (P1 module has fewer fields than it should be)

+ Request 

    + Body

            cadena=%238934075379000039321%2C%230%2CGPS%2C33.000%2C-3.234234%2C%230%2CP1%2C214%2C07%2C33f%2C%230%2CK1%2C0%24

+ Response 200

    + Body

            #0,GPS,#0,K1,0$


If the whole request is invalid:

+ Request

    + Body

           cadena=%238934075379000039321%2C

+ Response 400

    + Body
    
          {
             "reason":"malformed request",
             "details": ""
          }

When the device cannot be found

+ Request

    + Body
    
             cadena=%238934075379000039321%2C%230%2CGPS%2C33.000%2C-3.234234%2C%230%2CP1%2C214%2C07%2C33f%2C%230%2CK1%2C0%24

+ Response 404

    +Body

            {
                 "reason": "Device 8934075379000039321 not found",
                 "details": ""
            }








