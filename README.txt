compilar:
	gcc -ansi -pedantic -Wall main.c estado.c funcoes.c ./lib/list.c -o projecto

executar:
	./projecto

executar e direcionar dados por ficheiro:
	./projecto < 'nome ficheiro'

	exemplo:
	com o ficheiro chamado 'texte.txt', fazer:
	./projecto < teste.txt

fazer debugging da memória usada pelo programa:
	valgrind --tool=memcheck --leak-check=yes ./projecto < 'nome ficheiro'
	
	exemplo:
	com o ficheiro chamado 'texte.txt', fazer:
	valgrind --tool=memcheck --leak-check=yes ./projecto < teste.txt

Funcionalidades Implementadas:
	Todas.