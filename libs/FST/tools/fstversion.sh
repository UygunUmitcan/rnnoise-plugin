#!/bin/sh
#set -x

error() {
  echo "$*" 1>&2
}
usage() {
cat >/dev/stderr <<EOF
usage: $0 [<fstheader>]

Calculates the FST-version from parsing the <fstheader> file.
Changes the MINOR/MICRO version in <fstheader> and other files accordingly.
(The default <fstheader> file is "fst/fst.h")

the version is calculated as follows:
- the MINOR version is the number of all known/guessed symbols in the
  <fstheader>.
- if the MINOR version has increased, the MICRO version is set to '0'
- if the MINOR version has not changed, the MICRO version is incremented by '1'
- if the MINOR version has decreased, an error is output
EOF
}

fst_h="$1"
: "${fst_h:=fst/fst.h}"

fst_pc=extra/fst.pc

if [ ! -e "${fst_h}" ]; then
  usage
  error "'${fst_h}' does not exist" 1>&2
  exit 1
fi

extract_values() {
 grep -E "FST_(HOST|EFFECT|TYPE|CONST|FLAG|SPEAKER)"  "$1" \
	 | grep -E -v "# *define " \
	 | grep -v "FST_fst_h"
}

minor=$( \
extract_values "${fst_h}" \
| grep -v UNKNOWN \
| grep -c . \
)
unknown=$( \
extract_values "${fst_h}" \
| grep -E "FST_.*_UNKNOWN" \
| grep -c . \
)

major="$(grep '^\( *# *define  *FST_MAJOR_VERSION  *\)[0-9]*$' "${fst_h}" | sed -e 's|.*FST_MAJOR_VERSION *||')"
: "${major:=0}"
oldminor="$(grep '^\( *# *define  *FST_MINOR_VERSION  *\)[0-9]*$' "${fst_h}" | sed -e 's|.*FST_MINOR_VERSION *||')"
if [ "$((oldminor))" -lt "$((minor))" ]; then
  micro=0
elif [ "$((oldminor))" -gt "$((minor))" ]; then
  error "Number of symbols has decreased from ${oldminor} to ${minor}"
  exit 2
else
  micro="$(grep '^\( *# *define  *FST_MICRO_VERSION  *\)[0-9]*$' "${fst_h}" | sed -e 's|.*FST_MICRO_VERSION *||')"
  micro=$((micro+1))
fi

version="${major}.${minor}.${micro}"

sed \
	-e "s|^\( *# *define  *FST_MAJOR_VERSION  *\)[0-9]*$|\1${major}|" \
	-e "s|^\( *# *define  *FST_MINOR_VERSION  *\)[0-9]*$|\1${minor}|" \
	-e "s|^\( *# *define  *FST_MICRO_VERSION  *\)[0-9]*$|\1${micro}|" \
	-i "${fst_h}"
if [ -e "${fst_pc}" ]; then
  sed -e "s|^\(Version:\) [0-9]*\.[0-9]*\.[0-9]*|\1 ${major}.${minor}.${micro}|" -i "${fst_pc}"
fi
echo "version: ${major}.${minor}.${micro}"
echo "identifiers: known:${minor} unknown:${unknown}"
