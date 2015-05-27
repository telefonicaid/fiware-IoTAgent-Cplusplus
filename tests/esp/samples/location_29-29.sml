<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<sos:InsertObservation service="SOS" version="1.0.0" xmlns:sos="http://www.opengis.net/sos/1.0" xmlns:gml="http://www.opengis.net/gml" xmlns:om="http://www.opengis.net/om/1.0" xmlns:paid="urn:ogc:def:dictionary:PAID:1.0:paid" xmlns:sml="http://www.opengis.net/sensorML/1.0.1" xmlns:swe="http://www.opengis.net/swe/1.0.1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xlink="http://www.w3.org/1999/xlink">
    <sos:AssignedSensorId>id001</sos:AssignedSensorId>
    <om:Observation>
        <om:featureOfInterest />
        <om:samplingTime>
            <gml:TimeInstant frame="urn:x-ogc:def:trs:IDAS:1.0:ISO8601">
                <gml:timePosition frame="urn:x-ogc:def:trs:IDAS:1.0:ISO8601">2014-08-01T13:35:26Z</gml:timePosition>
            </gml:TimeInstant>
        </om:samplingTime>
        <om:procedure xlink:href="id001" />
        <om:observedProperty xlink:href="urn:x-ogc:def:phenomenon:IDAS:1.0:location" />
        <om:result>
            <swe:Position>
                <swe:location>
                    <swe:Vector definition="urn:x-ogc:def:phenomenon:IDAS:1.0:location">
                        <swe:coordinate name="latitude">
                            <swe:Quantity definition="urn:x-ogc:def:phenomenon:IDAS:1.0:latitude">
                                <swe:value>29</swe:value>
                                <swe:uom code="urn:x-ogc:def:uom:IDAS:1.0:degree" />
                            </swe:Quantity>
                        </swe:coordinate>
                        <swe:coordinate name="longitude">
                            <swe:Quantity definition="urn:x-ogc:def:phenomenon:IDAS:1.0:longitude">
                                <swe:value>29</swe:value>
                                <swe:uom code="urn:x-ogc:def:uom:IDAS:1.0:degree" />
                            </swe:Quantity>
                        </swe:coordinate>
                        <swe:coordinate name="altitude">
                            <swe:Quantity definition="urn:x-ogc:def:phenomenon:IDAS:1.0:altitude">
                                <swe:value>29</swe:value>
                                <swe:uom code="urn:x-ogc:def:uom:IDAS:1.0:meter" />
                            </swe:Quantity>
                        </swe:coordinate>
                    </swe:Vector>
                </swe:location>
            </swe:Position>
        </om:result>
    </om:Observation>
</sos:InsertObservation>
