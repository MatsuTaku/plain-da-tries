# Data sets are saved in GoogleCloud
Data sets that used researches are placed on GoogleDrive.
Please download it to current directory from follows:
https://drive.google.com/file/d/1fu5EUJJb9dyepDcUviGDE6WMSLbh2rZd/view?usp=sharing

or use script 'download.sh' which commands as follows::
```bash
FILE_ID=1fu5EUJJb9dyepDcUviGDE6WMSLbh2rZd
FILE_NAME=basic_data_sets.tar.xz
curl -sc /tmp/cookie "https://drive.google.com/uc?export=download&id=${FILE_ID}" > /dev/null
CODE="$(awk '/_warning_/ {print $NF}' /tmp/cookie)"
curl -Lb /tmp/cookie "https://drive.google.com/uc?export=download&confirm=${CODE}&id=${FILE_ID}" -o ${FILE_NAME}
```
