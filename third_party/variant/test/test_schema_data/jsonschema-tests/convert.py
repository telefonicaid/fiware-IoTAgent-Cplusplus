
import json
import sys

tests = json.load(sys.stdin)

out = []
for t in tests:
	for case in t['tests']:
		out.append({
				'name': t['description'] + ' -- ' + case['description'],
				'schema': t['schema'],
				'data': case['data'],
				'fail': not case['valid']
				})
json.dump(out, sys.stdout)
