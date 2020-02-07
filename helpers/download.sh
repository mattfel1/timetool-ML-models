#!/bin/bash
# this runs on lagos, you just need to provide a bunch of wget links

links=(https://file.io/ecc2ZQ https://file.io/GzFiUK https://file.io/UmJccx https://file.io/DSjOYO https://file.io/QO1npb https://file.io/mt4fPE https://file.io/RP95lV https://file.io/mFp4Fv https://file.io/yb309u https://file.io/R2AIu6 https://file.io/sTDeYy https://file.io/v40ijE https://file.io/XlkLri https://file.io/LJW6iQ https://file.io/jiXKOA https://file.io/Sm4aIk https://file.io/EJP9ly https://file.io/H6z9ga https://file.io/aHeUkt)
ctr=10000
for link in ${links[@]}; do
	curl $link -o file${ctr}.tar.bz
	ctr=$((ctr+5000))
done
