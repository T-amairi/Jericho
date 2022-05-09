import os

os.chdir(r".\topology\CSV")

def BalancedTreeToCSV(profondeur, nbr_de_fils, filename = "Tree"):
	the_file_i_guess = open(filename + '.csv', 'w+')

	counter = 0
	the_file_i_guess.write('0' + ',')
	for fils in range(nbr_de_fils - 1):
		the_file_i_guess.write(str(fils + 1) + ',')
	the_file_i_guess.write(str( nbr_de_fils ) + '\n')


	for etage in range(1, profondeur - 2):
		counter += nbr_de_fils**(etage - 1)
		for noeud in range(  nbr_de_fils**etage ):
			id = counter + noeud
			offset = id * nbr_de_fils + 1
			the_file_i_guess.write(str( id ) + ',' + str( (id-1)//3) + ',')

			for fils in range(nbr_de_fils - 1):
				the_file_i_guess.write(str(fils + offset) + ',')
			the_file_i_guess.write(str(nbr_de_fils - 1 + offset) + '\n')

	
		counter += nbr_de_fils**(profondeur - 3)
		for noeud in range(  nbr_de_fils**(profondeur - 2) ):
			id = counter + noeud
			offset = id * nbr_de_fils + 1
			the_file_i_guess.write(str( id ) + ',' + str( (id-1)//3) + ',')

			for fils in range(nbr_de_fils - 1):
				the_file_i_guess.write(str( -fils -offset) + ',')
			the_file_i_guess.write(str( -(nbr_de_fils - 1 + offset) )  + '\n' )


	counter += nbr_de_fils**(profondeur - 2)
	for noeud in range(  nbr_de_fils**(profondeur - 1) ):
		id = counter + noeud
		offset = id * nbr_de_fils + 1
		the_file_i_guess.write(str( -id ) + ','+ str( (id-1)//3) + '\n')

BalancedTreeToCSV(4,3)