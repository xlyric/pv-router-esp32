#!/bin/sh


gzip -9 index.html -c > index.html.gz
gzip -9 index-ap.html -c > index-ap.html.gz
gzip -9 index-light.html -c > index-light.html.gz
gzip -9 wifi.html -c > wifi.html.gz
gzip -9 envoy.html -c > envoy.html.gz

gzip -9 config.html -c > config.html.gz
gzip -9 config-ap.html -c > config-ap.html.gz
gzip -9 config-light.html -c > config-light.html.gz

gzip -9 mqtt.html -c > mqtt.html.gz

gzip -9 ../data2/sb-admin-2.min.css -c > sb-admin-2.min.css.gz
gzip -9 ../data2/all.min.css -c > all.min.css.gz

gzip -9 ../data2/jquery.min.js -c > jquery.min.js.gz
gzip -9 ../data2/bootstrap.bundle.min.js -c > bootstrap.bundle.min.js.gz
gzip -9 ../data2/bootstrap.bundle.min.js.map -c > bootstrap.bundle.min.js.map.gz
gzip -9 ../data2/sb-admin-2.js -c > sb-admin-2.js.gz
gzip -9 ../data2/fa-solid-900.woff2 -c > fa-solid-900.woff2.gz

gzip -9 log.html  -c > log.html.gz
