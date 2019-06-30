rm -r output
mkdir -p output;
techs="shm mmap zeromq domain fifo";
min_size_power=6
max_size_power=6
size_powers="$(seq $min_size_power $max_size_power)";
size_factors="$(seq 5 2 5)"
tries="$(seq 1 3)"
for tech in $techs; do
	for size_power in $size_powers; do
		for size_factor in $size_factors; do
			for try in $tries; do
				size=$(($size_factor*10**$size_power))
				count=10000
				echo "try $try: $tech -s $size -c $count";
				OUTPUT_FILE="output/${tech}_size-${size}_count-${count}_try-${try}.txt"
				./$tech/$tech -s $size -c $count > "$OUTPUT_FILE";
				sleep 1
				PS_CLIENT="start"
				PS_SERVER="start"
				while [[ -n "$PS_CLIENT" ]] && [[ -n "$PS_SERVER"  ]]
				do
					sleep 1
					PS_OUTPUT="$(ps -ef | grep "$tech[-]")"
					PS_CLIENT=$(echo $PS_OUTPUT | grep "client")
					PS_SERVER=$(echo $PS_OUTPUT | grep "server")
				done
				if [[ -n "$PS_CLIENT" ]] || [[ -n "$PS_SERVER"  ]]
				then
					echo $PS_CLIENT
					echo $PS_SERVER
					sleep 5
				fi
				killall -9 "$tech-client" &> /dev/null
				killall -9 "$tech-server" &> /dev/null

			done
		done; 
	done;
done
