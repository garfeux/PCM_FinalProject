#!/bin/bash

# Commande et fichier d'entrée
COMMAND="./tspcc"
INPUT_FILE="dj38.tsp"
ORIGINAL_DIMENSION=$(grep "DIMENSION" $INPUT_FILE | awk '{print $2}')

# Plage de valeurs pour le nombre de villes
MIN_CITIES=11
MAX_CITIES=19

# Plage et palier pour le nombre de threads
THREADS_MIN=1
THREADS_MAX=260
THREAD_STEP=8 # Incrément pour le nombre de threads

# Nombre d'exécutions par configuration pour la moyenne
ITERATIONS=1

# Fichier de sortie pour enregistrer les résultats
OUTPUT_FILE="execution_times.csv"

# En-tête pour le fichier de résultats
echo "Cities,Threads,Execution Time (Average)" > $OUTPUT_FILE

# Boucle sur le nombre de villes
for cities in $(seq $MIN_CITIES $MAX_CITIES)
do
    echo "Testing with CITIES=$cities..."

    # Modifier le fichier pour ajuster le nombre de villes
    sed -i.bak "s/^DIMENSION: .*/DIMENSION: $cities/" $INPUT_FILE

    # Boucle sur le nombre de threads avec un pas défini
    for threads in $(seq $THREADS_MIN $THREAD_STEP $THREADS_MAX)
    do
        echo "  THREADS_TO_USE=$threads..."

        total_time=0

        # Effectuer plusieurs itérations
        for ((i=1; i<=ITERATIONS; i++))
        do
            # Exécuter la commande et capturer la sortie
            output=$($COMMAND $INPUT_FILE $threads)

            # Extraire le temps depuis la sortie de la commande
            time=$(echo "$output" | grep "Time:" | awk '{print $2}')

            # Vérifier si une valeur a été extraite
            if [[ -z "$time" ]]; then
                echo "Erreur : Impossible d'extraire le temps pour CITIES=$cities, THREADS=$threads à l'itération $i."
                exit 1
            fi

            # Ajouter le temps à la somme totale
            total_time=$(echo "$total_time + $time" | bc)
        done

        # Calculer la moyenne
        average_time=$(echo "scale=4; $total_time / $ITERATIONS" | bc)

        # Enregistrer les résultats
        echo "$cities,$threads,$average_time" >> $OUTPUT_FILE
        echo "CITIES=$cities, THREADS=$threads -> Average Time: $average_time seconds"
    done
done

# Restaurer le fichier original
mv $INPUT_FILE.bak $INPUT_FILE

# Indiquer que le script est terminé
echo "Test complet. Résultats enregistrés dans $OUTPUT_FILE."
