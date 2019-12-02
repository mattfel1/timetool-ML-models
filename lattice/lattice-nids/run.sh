
learning_rate=( 0.02 )
num_lattices=( 32 )

mkdir evaluations

for lr in "${learning_rate[@]}"; do
	for nl in "${num_lattices[@]}"; do
		msg="Building eval: Learning Rate $lr | Number of lattices $nl"
		test_set="LR_${lr}_NL_${nl}"
		mkdir evaluations/${test_set}
		echo $msg
		python nids-kdd.py  --create_quantiles --learning_rate=$lr --num_lattices=$nl --output_dir=evaluations/${test_set}/output/ --quantiles_dir=evaluations/${test_set}/  &> evaluations/${test_set}/status_train_${test_set}.txt &
	done
done
wait

for lr in "${learning_rate[@]}"; do
	for nl in "${num_lattices[@]}"; do
		msg="Evaluating: Learning Rate $lr | Number of lattices $nl"
		test_set="LR_${lr}_NL_${nl}"
		echo $msg
		python nids-kdd.py  --learning_rate=$lr --num_lattices=$nl --output_dir=evaluations/${test_set}/output/ --quantiles_dir=evaluations/${test_set}/ --run=evaluate &> evaluations/${test_set}/status_evaluate_${test_set}.txt &
	done
done
wait


for lr in "${learning_rate[@]}"; do
	for nl in "${num_lattices[@]}"; do
		msg="Timing: Learning Rate $lr | Number of lattices $nl"
		test_set="LR_${lr}_NL_${nl}"
		echo $msg
		python nids-kdd.py  --learning_rate=$lr --num_lattices=$nl --output_dir=evaluations/${test_set}/output/ --quantiles_dir=evaluations/${test_set}/ --run=time &> evaluations/${test_set}/status_timing_${test_set}.txt &
	done
done
wait

