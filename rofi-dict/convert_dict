#!/usr/bin/env python

# Used to convert dictionaries from https://github.com/freedict/fd-dictionaries
# from .tei to .json format.
#
# Usage: convert_dict input_dict.tei output_dict.json

from xml.etree.ElementTree import ElementTree
import json
import sys
import os

if len(sys.argv) != 3:
    print("Usage: convert_dict input_dict.tei output_dict.json")
    sys.exit(1);

if not os.path.isfile(sys.argv[1]):
    print("Could not find input file.")
    sys.exit(1);

try:
    json_file = open(sys.argv[2], 'w')
except OSError:
    print("Could not open output file.")
    sys.exit(1);

elem_tree = ElementTree();
elem_tree.parse(sys.argv[1]);
tree = elem_tree.getroot()[1][0];

results = dict();

for entry in tree:
    if len(entry) > 0 and len(entry[0]) > 0:
        senses = list();

        for index, sense in enumerate(entry.findall("{http://www.tei-c.org/ns/1.0}sense")):
            cits = sense.findall("{http://www.tei-c.org/ns/1.0}cit")

            if len(cits) > 0:
                temp = list()
                for cit in cits:
                    if cit[0].text is not None:
                        temp.append(cit[0].text);
                if len(temp) > 0:
                    senses.append(", ".join(temp))

        if len(senses) > 0:
            translation = "; ".join(senses);
            results.update({entry[0][0].text : translation});

json.dump(results, json_file, sort_keys = True)
json_file.close()
