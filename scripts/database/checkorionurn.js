/**
 *  script to check old data from sbc in orion
 *  look for urn in entities
 *  version 0.1
 *
 *  use_: mongo host:port/database  < checkorionurn.js
 */

function checkAllAtrNames() {
  db.entities.distinct("attrNames")
      .forEach(function(attn) { checkField(attn); });
}

function checkField(attrName) {
  var name = "attrs." + attrName + ".md.type";
  var value = "/^urn:/";
  var query = {};
  query[name] = value;

  db.entities.find(query).forEach(function(d) {
    try {
      d.attrs[attrName].md.forEach(function(meta) {
        if (meta.type.substring(0, 4) == 'urn:') {
          removeMD(d._id, attrName, meta);
        }

      });
    } catch (err) {
      print(err);
    }
  });
  print("updated " + num_devices_update + " devices");
}

// db.entities.update({_id},  {$pull : { "attrs.fillLevel.md" : {}  } }  );
function removeMD(id, attrName, md) {
  var name = "attrs." + attrName + ".md";
  var attrs = {};
  attrs[name] = md;

  db.entities.update(id, {$pull : attrs});
}

print "start check data consistency";
checkAllAtrNames();

print "end check data consistency";
