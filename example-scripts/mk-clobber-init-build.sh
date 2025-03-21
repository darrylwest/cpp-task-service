#!/usr/bin/env bash
# dpw@larkspur.localdomain
# 2025-03-21 19:52:50
#

set -eu

export host='alamo:2032'

curl -k -d './mk pull clobber init build test' -XPOST "https://${HOST}/queue"


