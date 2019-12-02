
learning_rate=( 0.02 )
num_lattices=( 16 32 )



for lr in "${learning_rate[@]}"; do
	for nl in "${num_lattices[@]}"; do
		msg="Timing: Learning Rate $lr | Number of lattices $nl"
		test_set="LR_${lr}_NL_${nl}"
		echo $msg
		python nids-kdd.py  --learning_rate=$lr --num_lattices=$nl --type=simplex --output_dir=simplex_evaluations/${test_set}/output/ --quantiles_dir=simplex_evaluations/ --run=time &> simplex_evaluations/${test_set}/status_timing_${test_set}.txt &
	done
done
wait

