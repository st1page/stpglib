# simple file storage

## structuer of sfs file

file header  
table1  
table2  
table3  
...

### sfs file header

All values little-endian
The "offset" is measured from the start of the file

start|size| description
------|----|----
00| 2B| Magic number(0x534653aa,SFS.)
02| 1B| Version number
03| 1B| Pad byte
04| 4B| CRC-32
08| 4B| FIle size
0c| 4B| Reserved
10| 1B| The number of tables
11| 3B| Pad bytes
14|64B| tables offset(uint32[16])

### Table 

table header  
record1  
record2  
...  
free space  
...  
Varchar2  
Varchar1  
record metadata

### Table header

All values little-endian
The "offset" is measured from the start of the table

start|size| description
------|----|----|
00| 4B| total size of the Table
04| 4B| Free space left in the table
08| 4B| Number of Varchars in the table
0c| 4B| The last Varchar's offset
10| 4B| Number of Records in the table
14| 4B| Record metadata offset

### Record metadata
start|size| description
------|----|----
04| 4B| the number of fields
08| 1B*fieldsNum | the length/type of fields 

the length of fields is a unsigned int8 means the Bytes of the fields  
if it is 0, the fields is the offset of a varchar, which is measured from the start of the table
