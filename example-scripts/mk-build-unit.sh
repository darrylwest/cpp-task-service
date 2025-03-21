#!/usr/bin/env bash
# dpw@larkspur.localdomain
# 2025-03-21 19:49:11
#

set -eu

export host='alamo:2032'

curl -k -d './mk pull build unit'  -XPOST "https://${HOST}/queue"


