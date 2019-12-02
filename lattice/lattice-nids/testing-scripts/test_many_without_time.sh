
learning_rate=( 0.002 0.02 )
num_lattices=( 64 128 )


for lr in "${learning_rate[@]}"; do
	for nl in "${num_lattices[@]}"; do
		msg="Building eval: Learning Rate $lr | Number of lattices $nl"
		test_set="LR_${lr}_NL_${nl}"
		mkdir simplex_evaluations/${test_set}
		echo $msg
		python nids-kdd.py --learning_rate=$lr --num_lattices=$nl --type=simplex --output_dir=simplex_evaluations/${test_set}/output/ --quantiles_dir=simplex_evaluations/  &> simplex_evaluations/${test_set}/status_train_${test_set}.txt &
	done
done
wait

for lr in "${learning_rate[@]}"; do
	for nl in "${num_lattices[@]}"; do
		msg="Evaluating: Learning Rate $lr | Number of lattices $nl"
		test_set="LR_${lr}_NL_${nl}"
		echo $msg
		python nids-kdd.py  --learning_rate=$lr --num_lattices=$nl --type=simplex --output_dir=simplex_evaluations/${test_set}/output/ --quantiles_dir=simplex_evaluations/ --run=evaluate &> simplex_evaluations/${test_set}/status_evaluate_${test_set}.txt &
	done
done
wait


