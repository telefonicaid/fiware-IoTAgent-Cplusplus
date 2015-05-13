/**
 *  script to check data consistency  in mongo  iotagent
 *  version 0.1
 *
 *  use_: mongo host:port/database  < checkconsistencydata.js
 */


function getProtocolName(resource) {
  if (resource == "/iot/tt") {
    return "PDI-IoTA-ThinkingThings";
  }
  else if (resource == "/iot/mqtt") {
    return "PDI-IoTA-MQTT-UltraLight";
  }
  else if (resource == "/iot/d") {
    return "PDI-IoTA-UltraLight";
  }
  else {
    print("no exists translate for resource:" + resource);
  }
  return "";
}


function checkProtocolInDevice() {
  num_devices_update =0;
  print("review data model to check if all devices have protocol");
  db.DEVICE.find({"protocol":{ $exists: false}}).forEach(function(d) {
    try {
      resourceObj = db.SERVICE.findOne({service:d.service, service_path: d.service_path});
      if (resourceObj == null) {
        print("no exists service for device:" + d.device_id + "|service:" +
              d.service  + "|service_path:" + d.service_path);
      }
      else {
        resource = getProtocolName(resourceObj.resource);
        if (resource != ""){
           db.DEVICE.update({_id:d._id}, {$set:{"protocol": resource}});
        }
        num_devices_update++;
      }
    }
    catch (err) {
      print(err);
    }
  });
  print("updated " + num_devices_update + " devices");
}


print "starting checking data consistency";

checkProtocolInDevice()

print "end check data consistency";
