#!/bin/bash

# Commande et fichier d'entrée
COMMAND="./tspcc"
INPUT_FILE="dj38.tsp"
ORIGINAL_DIMENSION=$(grep "DIMENSION" $INPUT_FILE | awk '{print $2}')

# Plage de valeurs pour le nombre de villes
MIN_CITIES=17
MAX_CITIES=17

# Plage et palier pour le nombre de threads
THREADS_MIN=32
THREADS_MAX=256
THREAD_STEP=32 # Incrément pour le nombre de threads

# Maximum depth qui est dans la queue
MAX_DEPTH_MIN=10
MAX_DEPTH_MAX=14
MAX_DEPTH_STEP=2


# Nombre d'exécutions par configuration pour la moyenne
ITERATIONS=2

# Fichier de sortie pour enregistrer les résultats
OUTPUT_FILE="execution_times.csv"

# En-tête pour le fichier de résultats
echo "Cities,Threads,Max depth,Execution Time (Average),Standard_deviation" > $OUTPUT_FILE

# Boucle sur le nombre de villes
for cities in $(seq $MIN_CITIES $MAX_CITIES)
do
    echo "Testing with CITIES=$cities..."

    # Modifier le fichier pour ajuster le nombre de villes
    sed -i.bak "s/^DIMENSION: .*/DIMENSION: $cities/" $INPUT_FILE

    for max_depth in $(seq $MAX_DEPTH_MIN $MAX_DEPTH_STEP $MAX_DEPTH_MAX )
    do

        # Boucle sur le nombre de threads avec un pas défini
        for threads in $(seq $THREADS_MIN $THREAD_STEP $THREADS_MAX)
        do
            echo "  THREADS_TO_USE=$threads..."

            total_time=0
            total_standard_deviation=0

            # Effectuer plusieurs itérations
            for ((i=1; i<=ITERATIONS; i++))
            do
                # Exécuter la commande et capturer la sortie
                echo "$COMMAND $INPUT_FILE $threads $max_depth"
                output=$($COMMAND $INPUT_FILE $threads $max_depth)

                # Extraire le temps depuis la sortie de la commande
                time=$(echo "$output" | grep "Time:" | awk '{print $3}')
                standard_deviation=$(echo "$output" | grep "Standard deviation:" | awk '{print $3}')

                # Vérifier si une valeur a été extraite
                if [[ -z "$time" ]]; then
                    echo "Erreur : Impossible d'extraire le temps pour CITIES=$cities, THREADS=$threads à l'itération $i."
                    exit 1
                fi

                if [[ -z "$standard_deviation" ]]; then
                    echo "Erreur : Impossible d'extraire l'écart-type pour CITIES=$cities, THREADS=$threads à l'itération $i."
                    exit 1
                fi

                # Ajouter le temps à la somme totale
                total_time=$(echo "$total_time + $time" | bc)
                total_standard_deviation=$(echo "$total_standard_deviation + $standard_deviation" | bc)

            done

            # Calculer la moyenne
            average_time=$(echo "scale=4; $total_time / $ITERATIONS" | bc)
            average_standard_deviation=$(echo "scale=4; $standard_deviation / $ITERATIONS" | bc)

            # Enregistrer les résultats
            echo "$cities,$threads,$max_depth,$average_time,$average_standard_deviation" >> $OUTPUT_FILE
            echo "CITIES=$cities, THREADS=$threads, MAX_DEPTH=$max_depth -> Average Time: $average_time seconds"
        done
    done
done

# Restaurer le fichier original
mv $INPUT_FILE.bak $INPUT_FILE

# Indiquer que le script est terminé
echo "Test complet. Résultats enregistrés dans $OUTPUT_FILE."
