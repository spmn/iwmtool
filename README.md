# iwmtool
Tool for manipulating mpdata files for CoD4

### Quick info:
CoD4 stores multiplayer stats in an encrypted file. This tool is able to encrypt and decrypt the stats file, and is also able to edit stats.

### Supported operations:
* convert vanilla `iwm0` format to CoD4X `ice0` format and vice-versa (basically: encryption and decryption)
* simple stats manipulation

### Example:
* Decryption:
```console
$ iwmtool --mode=decrypt --input=mpdata --output=mpdata_decrypted --key=YOUR_CD_KEY
```

* Encryption:
```console
$ iwmtool --mode=encrypt --input=mpdata --output=mpdata_encrypted --key=YOUR_CD_KEY
```

* Edit stats: (set total kills to 123)
```console
$ iwmtool --mode=stats --input=mpdata --output=mpdata_edited --index=2303 --set=123
```

*Note:* Most stats indices can be found in string tables and game scripts extracted from `*_mp.ff` files.

*Note:* You can omit CD-key if CoD4 is properly installed on your PC or if you edit a decrypted mpdata file.
