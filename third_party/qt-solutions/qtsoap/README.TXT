Qt Solutions Component: SOAP

The Qt SOAP project provides basic web service support with
version 1.1 of the SOAP protocol.



Notes:

This is a partial implementation of SOAP v1.1.

- Server side SOAP is not supported. - References to values (id &
href attributes) are not supported. - Only arrays with less than 5
dimensions are supported. - Namespaces for types are not checked.
Only the type names are used. - The encodingStyle attribute is
ignored. The serialization and encoding rules from section 5 in
the SOAP v1.1 specification are assumed regardless of the value of
the encodingStyle attribute. - QtSoapType does not have accessors
for attributes, which means for example that actor, mustUnderstand
and so on are not accessible in headers. - The SOAP root attribute
is not supported.

Version history:

1.1: - Have setContent() always skip first entry if it's not an
     element

1.2: - QtSoapArray::count() returns correct count

1.3: - Documentation fixes

2.0: - Version 1.3 ported to Qt 4.

2.1: - Fix memory leak, and fix for Qt 4.1.

2.2: - Bugfix, and fix for Qt 4.2.

2.3: - Handle (but don't preserve) comments in structs/arrays
     - Fix memory leak for QtSoapArray
     - Allow Array types to not have the "type" attribute set, as per
     the Soap 1.1 spec
     - Handle fault code of type (in addition to type String)

2.4: - Fix: Use UTF8 encoding, as the standard recommends, instead
     of Latin1. Indicate the charset in the header.
     - Fix: Allow header items to be added before body items, so the
     output will have the correct order (header before body), as per
     the standard.
     - Fix: Possible memory leak when copying QtSoapStructs and
     -Arrays.

2.5: - Fixes: Error handling. responseReady signal would sometimes
     not be emitted in error situations. Now, always emitted, and the
     error is recorded in the response.
     - Fixes: node parsing bug.
     - Fixes: arrayiterator bug, and wrong indices in array example
     code in doc.
     - Fixes: copying structs and arrays.
     - Added: support operator[] for structs.
     - Allow the transport to be set to https mode.

2.6: - Misc. minor fixes.
     - LGPL release.

2.7: - Auto-generation in SoapNamespaces has never worked as
     intended, dropped.
     - The HttpTransport class is now based on QNetworkManager et al.
     instead of QHttp. This entails some minor API changes to that
     class.

