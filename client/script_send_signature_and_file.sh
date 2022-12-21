#!/usr/bin/env bash
signatureFile="${1}"
scriptFile="${2}"
username="${3}"
mkdir $username
od -An -vtx1 $signatureFile > Check.txt
cat Check.txt > $username/file_to_send && cat $scriptFile >> $username/file_to_send
rm Check.txt