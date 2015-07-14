
# Authentication, Authorization and PEP Rules
## Configuration
An IoTAgent could be configured to use an authentication and authorization system in northbound API, i.e, provision API. In order to use this feature, you must include this information in configuration file (values are shown as examples):

```
"oauth": {
    "on_behalf_trust_url":"http://10.95.82.233:5001/v3/auth/tokens",
    "validate_token_url": "http://10.95.82.233:5001/v3/auth/tokens",
    "get_roles_url": "http://10.95.82.233:5001/v3/role_assignments",
    "retrieve_subservices_url": "http://10.95.82.233:5001/v3/projects",
    "access_control_url": "http://10.95.82.233:7070",
    "pep_user": "pep",
    "pep_password": "pep",
    "pep_domain": "admin_domain",
    "on_behalf_user": "iotagent",
    "on_behalf_password": "iotagent",
    "timeout": 5
 }
 ```

PEP feature is ONLY supported with Openstack Keystone.
Every operation over Provision API is supposed having three HTTP headers:
- X-Auth-Token: user token when user is logged into Keystone system.
- Fiware-Service: service belonging user.
- Fiware-ServicePath: subservice into service.

## Authentication
In order to get its access token, a user can send the following request to Keystone:
```
curl http://localhost:5000/v3/auth/tokens \
    -s \
    -i \
    -H "Content-Type: application/json" \
    -d '
{
    "auth": {
        "identity": {
            "methods": [
                "password"
            ],
            "password": {
                "user": {
                    "domain": {
                        "name": "SmartCity"
                    },
                    "name": "alice",
                    "password": "password"
                }
            }
        }
    }
}'
```
The token can be found in the `X-Subject-Token` header of the response:
```
X-Subject-Token: MIIC3AYJKoZIhvcNAQcCoIICzTCCAskCAQExCTAHBgUrDgMCGjCCATIGCSqGSIb3DQEHAaCCASMEggEfeyJ0b2tlbiI6IHsiaXNzdWVkX2F0IjogIjIwMTQtMTAtMTBUMTA6NTA6NDkuNTMyNTQyWiIsICJleHRyYXMiOiB7fSwgIm1ldGhvZHMiOiBbInBhc3N3b3JkIl0sICJleHBpcmVzX2F0IjogIjIwMTQtMTAtMTBUMTE6NTA6NDkuNTMyNDkxWiIsICJ1c2VyIjogeyJkb21haW4iOiB7ImlkIjogImY3YTViOGUzMDNlYzQzZThhOTEyZmUyNmZhNzlkYzAyIiwgIm5hbWUiOiAiU21hcnRWYWxlbmNpYSJ9LCAiaWQiOiAiNWU4MTdjNWUwZDYyNGVlNjhkZmI3YTcyZDBkMzFjZTQiLCAibmFtZSI6ICJhbGljZSJ9fX0xggGBMIIBfQIBATBcMFcxCzAJBgNVBAYTAlVTMQ4wDAYDVQQIDAVVbnNldDEOMAwGA1UEBwwFVW5zZXQxDjAMBgNVBAoMBVVuc2V0MRgwFgYDVQQDDA93d3cuZXhhbXBsZS5jb20CAQEwBwYFKw4DAhowDQYJKoZIhvcNAQEBBQAEggEAKRGV3uu8fiS7UNm47KhltSjlY1e7KnedUcD-mdwz6Asbo7X9hbtljy1ml9gGcuMf6vX4tycx4goRyMARPS7YKROd0evZtnYArIyx0IrmwDaqodwp8BxBCxFgHRZtCwzHvZFEaUcClydQq7HJvBfTgTwH4v1aJkMyK8wLMP-CYyiZSfCIWPVnoB9I3P56jeKHkmcryYLgT2I-AwDBj1zd9HPzUjyQuNj5rCMkJjvz-A9-hef6AMMZuYPMIYdkei+deq86O1qFuo7PpO2SA7QWkqjcsKs9v+myvHhLrBre9GLP2hP1rc4D67lSL2XB1UY20mc6FNIVIErxT0DOSXltXQ==
Vary: X-Auth-Token
Content-Type: application/json
Content-Length: 287
Date: Fri, 10 Oct 2014 10:50:49 GMT

{
  "token": {
    "issued_at": "2014-10-10T10:50:49.532542Z",
    "extras": {},
    "methods": [
      "password"
    ],
    "expires_at": "2014-10-10T11:50:49.532491Z",
    "user": {
      "domain": {
        "id": "f7a5b8e303ec43e8a912fe26fa79dc02",
        "name": "SmartValencia"
      },
      "id": "5e817c5e0d624ee68dfb7a72d0d31ce4",
      "name": "alice"
    }
  }
}
```

For details on user and role creation, check the Keystone API.

## Authorization
Once the user is authenticated, the IoTAgent will ask the Access Control for its permissions. In order for the request to be accepted, at least one rule has to match the request information and the user roles.

Rules are defined in [XACML](https://www.oasis-open.org/committees/xacml/). The particular rules will depend on each case and are left to the authorization designer. The following document shows a typical rule explained for the use case of a IoT Agent:

```
<Policy xsi:schemaLocation="urn:oasis:names:tc:xacml:3.0:core:schema:wd-17
    http://docs.oasis-open.org/xacml/3.0/xacml-core-v3-schema-wd-17.xsd"
        PolicyId="policy03"
        RuleCombiningAlgId="urn:oasis:names:tc:xacml:3.0:rule-combining-algorithm:deny-unless-permit"
        Version="1.0" xmlns="urn:oasis:names:tc:xacml:3.0:core:schema:wd-17"
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

  <Target>
    <AnyOf>
      <AllOf>
        <Match MatchId="urn:oasis:names:tc:xacml:1.0:function:string-regexp-match">
          <AttributeValue
              DataType="http://www.w3.org/2001/XMLSchema#string"
              >fiware:iotagent:SmartValencia:/Foo:*</AttributeValue>
          <AttributeDesignator
              AttributeId="urn:oasis:names:tc:xacml:1.0:resource:resource-id"
              DataType="http://www.w3.org/2001/XMLSchema#string"
              MustBePresent="true"
              Category="urn:oasis:names:tc:xacml:3.0:attribute-category:resource" />
        </Match>
      </AllOf>
    </AnyOf>
  </Target>

  <Rule RuleId="policy03rule01" Effect="Permit">

    <Condition>
      <Apply FunctionId="urn:oasis:names:tc:xacml:1.0:function:string-equal">
        <Apply FunctionId="urn:oasis:names:tc:xacml:1.0:function:string-one-and-only">
          <AttributeDesignator
              AttributeId="urn:oasis:names:tc:xacml:1.0:action:action-id"
              DataType="http://www.w3.org/2001/XMLSchema#string"
              MustBePresent="true"
              Category="urn:oasis:names:tc:xacml:3.0:attribute-category:action" />
        </Apply>
        <AttributeValue
            DataType="http://www.w3.org/2001/XMLSchema#string"
            >read</AttributeValue>
      </Apply>
    </Condition>
  </Rule>

</Policy>

```

All the rules are associated to a service ID (the value of the `Fiware-Service` header) and a subservice. When the request arrives to the Access Control, the later will retrieve all the permissions for the user roles, each one represented by a XACML policy. All the policies are applied then in order to find any that would let the request be executed.

In the example, the policy states the following: "if the resource has the prefix `fiware:iotagent:SmartValencia:/Foo:` and the action `read` the request would be allowed". This policy will allow read access over all the resources in subservice `/Foo` of the service `SmartValencia` to the roles that have it assigned. The meaning of the term resource will depend on the component which is being protected by the particular access rules.

Another example could be this the following:

```
<Policy xsi:schemaLocation="urn:oasis:names:tc:xacml:3.0:core:schema:wd-17
    http://docs.oasis-open.org/xacml/3.0/xacml-core-v3-schema-wd-17.xsd"
        PolicyId="policy02"
        RuleCombiningAlgId="urn:oasis:names:tc:xacml:3.0:rule-combining-algorithm:deny-unless-permit"
        Version="1.0" xmlns="urn:oasis:names:tc:xacml:3.0:core:schema:wd-17"
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

  <Target>
    <AnyOf>
      <AllOf>
        <Match MatchId="urn:oasis:names:tc:xacml:1.0:function:string-equal">
          <AttributeValue
              DataType="http://www.w3.org/2001/XMLSchema#string">admin</AttributeValue>
          <AttributeDesignator
              AttributeId="urn:oasis:names:tc:xacml:1.0:subject:subject-id"
              DataType="http://www.w3.org/2001/XMLSchema#string"
              MustBePresent="false"
              Category="urn:oasis:names:tc:xacml:1.0:subject-category:access-subject" />
        </Match>
        <Match MatchId="urn:oasis:names:tc:xacml:1.0:function:string-regexp-match">
          <AttributeValue
              DataType="http://www.w3.org/2001/XMLSchema#string">fiware:iotagent:SmartValencia:/Foo:*</AttributeValue>
          <AttributeDesignator
              AttributeId="urn:oasis:names:tc:xacml:1.0:resource:resource-id"
              DataType="http://www.w3.org/2001/XMLSchema#string"
              MustBePresent="true"
              Category="urn:oasis:names:tc:xacml:3.0:attribute-category:resource" />
        </Match>
      </AllOf>
    </AnyOf>
  </Target>

  <Rule RuleId="policy02rule01" Effect="Permit">

    <Condition>
      <Apply FunctionId="urn:oasis:names:tc:xacml:1.0:function:string-equal">
        <Apply FunctionId="urn:oasis:names:tc:xacml:1.0:function:string-one-and-only">
          <AttributeDesignator
              AttributeId="urn:oasis:names:tc:xacml:1.0:action:action-id"
              DataType="http://www.w3.org/2001/XMLSchema#string"
              MustBePresent="true"
              Category="urn:oasis:names:tc:xacml:3.0:attribute-category:action" />
        </Apply>
        <AttributeValue
            DataType="http://www.w3.org/2001/XMLSchema#string">write</AttributeValue>
      </Apply>
    </Condition>
  </Rule>

</Policy>

```

In this example, only those users with `subjectId` (user's role) "admin" may write on resources of tenant SmartValencia and subservice /Foo.

Any number of policies can be included in the Access Control for each pair (tenant, subject). If any of the policies can be applied to the request and `Permit` the request, then the global result is a `Permit`. If none of the policies can be applied (no target exist for the tenant, subservice and subject of the request) the result will be `NotApplicable`. If there are policies that can be applied but all of them deny the access, the result will be a `Deny`.


## PEP Rules and Access Control resources

### IoT Agent resources

Resources applying PEP rules:
`
fiware:iotagent:{Fiware-Service}:{Fiware-ServicePath}:{uri-no-base}
`
For example, a pair Fiware-Service - Fiware-ServicePath `SmartCity - /Foo` getting all devices creates a resource like that:
`fiware:iotagent:SmartCity:/Foo:/devices`
Following this format, Access Control could defines permissions for every resource.

IoT Agent creates actions for every resource.
The following tables show the rules for detemining the action based on method and path of the request.

| Method | Path                               | Action |
|--------|------------------------------------|--------|
|  POST  |  /devices              | create |
|  GET  |   /devices              | read |
|  GET  |   /devices/{device-id}              | read |
|  PUT  |   /devices/{device-id}              | update |
|  DELETE | /devices/{device-id}             | delete |
|  POST  |  /services              | create |
|  GET  |   /services              | read |
|  PUT  |   /services              | update |
|  DELETE | /services             | delete |
|  POST | /ngsi/{uri-protocol}/updateContext | create |
|  POST | /ngsi/{uri-protocol}/queryContext | read |

_uri_protocol_ is the path following url base for IoT Agent in resource (view [IoT Agent Configuration](deploy.md)).

You can define actions in IoT Agent Configuration file following this format:
`
 "pep_rules": [
      {
          "verb": "POST",
          "action": "read",
          "uri": "/ngsi/<protocol>/queryContext"
       },
        {
          "verb": "POST",
          "action": "create",
          "uri": "/ngsi/<protocol>/updateContext"
        }
     ]
`
`verb` is HTTP method, `uri` is URI in HTTP request (you can see, url-basename is ommited). `<` and `>` are used to defien a variable element in uri.

