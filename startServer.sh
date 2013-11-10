output=$(ifconfig | awk ' {
if (NR%10 == 2) {
str=$2
split(str,arr,":")
print(arr[2])
}
}')
output=$(while IFS=';' read -ra ADDR; do
    for i in "${ADDR[@]}"; do
        awk -v "givenip=$i" ' { 
ipaddr_port = $0
split(ipaddr_port,ipaddr,":")
if (ipaddr[1]==givenip) {
print(NR-1)
}
}' "FileMesh.cfg"
    done
done <<< "$output")

while IFS=';' read -ra ADDR; do
    for i in "${ADDR[@]}"; do
	echo Starting server number $i
	./server $i &
    done
done <<< "$output"
