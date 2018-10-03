Example command line usages.
----------------------------

$ja-coap-client -m GET coap://192.168.0.100:56775/oic/res


For multicast request
----------------------

$ja-coap-client --non -m GET coap://224.0.1.187:5675/oic/res

$ja-coap-client --non -m GET coap://[ff02::158]:5675/oic/res

MULTICAST Address for coap
--------------------------

IPV4
----
224.0.1.187:5683

IPV6
----
FF02::158:5683
FF03::158:5683
FF04::158:5683