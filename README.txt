i)O projeto tem as seguintes directorias:
	Projeto1: Utilizado apenas com a funcionalidade do doTest.sh
	Projeto2: Aonde esta o projeto em si e o doTest.sh
		.CircuitSolver-ParSolver: Aonde se encontra o ParSolver e o doTest.sh
			. inputs: Inputs com que o ParSolver é utilizado.
			. results: Aonde se encontram os ficheiros com a extensao speedups.csv
		.lib: Ficheiros dados pelos docentes que compilam no ParSolver.
ii)Detalhes de utilizacão:
	Compilação do programa:
		Basta apenas utilizar make na diretoria CircuitSolver-ParSolver.
	doTest.sh:
		É utilizado na direitoria CircuitSolver-ParSolver com ./doTest.sh N FILE
		N é o numero Maximo de threads
		FILE é o ficheiro que se quer abrir
iii)CPU cores : 2
Clock Rate:  2592.000 Mhz no Processor 0 e 1
Modelo: Intel(R) Core(TM) i7-6500U CPU @ 2.50GHz
Informacao do uname -a : Linux miguelmota-VirtualBox 4.13.0-46-generic #51-Ubuntu SMP Tue Jun 12 12:36:29 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux
iv) KNOWN ISSUES:
Existe um pequeno bug grafico no doTest.sh.
Ele por vezes, ao escrever no ficheiro.speedups.csv, não mete em uma coluna, seja a dos exec_time ou a dos speedups, de meter uma virgula a seguir ao seu primeiro elemento.
Por exemplo: 8428274, quer dizer 8,428274.
Adicionei uns echos ao doTest.sh para mostrar que ele calcula tudo bem e deteta tudo bem, apenas não os metes bem no respetivo speedups.csv.

