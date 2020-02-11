#!/bin/bash
# this runs on pskevin after you have a bunch of zips (i.e. ran copydata.sh)

pushd /u1/mattfel/zips
for f in ./*; do
  echo $f
  f=`echo $f | sed "s/\.\///g"`
  #curl --upload-file $f https://transfer.sh/$f
  curl -F "file=@$f" https://file.io
  echo -e "\n---"
done

popd

