
output_file="script_output.txt"
path=/pkg

for (( i=1; i<101; i++ ))
do
  time=$( TIMEFORMAT="%R"; { time ./mdu -j "$i" $path 2> /dev/null; } 2>&1)
  echo -n "$time">> "$output_file"
  echo -n "$time"
done
