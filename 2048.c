#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <ctype.h>
#include <time.h>
#include <sys/select.h>

#define FOREVER         1
#define INIT_ROW        5
#define INIT_COL        10
#define MAX1            2
#define MAX2            4
#define S_TO_WAIT	3
#define MILIS_TO_WAIT 	0
#define KEYBOARD	0
#define SELECT_EVENT	1
#define SELECT_NO_EVENT	0

void meniu(WINDOW *wnd, int **a, int **b, int *punctaj);

void creare_tabel(WINDOW *wnd)
{
	int row = INIT_ROW, col = INIT_COL;
	wclear(wnd); //sterge ecran;
	wrefresh(wnd); //update fereastra;
	box(wnd, 0, 0); //border fereastra;
	//legenda;
	mvwprintw(wnd, row, col, "%s", "Indicati deplasarea celulelor folosind tastele: ");
	mvwprintw(wnd, row+1, col+2, "%s", "W - sus ");
	mvwprintw(wnd, row+2, col+2, "%s", "S - jos ");
	mvwprintw(wnd, row+3, col+2, "%s", "A - dreapta ");
	mvwprintw(wnd, row+4, col+2, "%s", "D - stanga ");
	mvwprintw(wnd, row+6, col+0, "%s", "Pentru iesire, apasati tasta Q.");
	mvwprintw(wnd, row+5, col+2, "%s", "U - undo ");
}

void coloreaza(WINDOW *wnd, int row, int col) //coloreaza patratica din tabela asociata unui numar;
{
	int k;
	for(k = -1; k <= 1; k++)
	{
		mvwprintw(wnd, row+k, col-3, "%c", ' ');
		mvwprintw(wnd, row+k, col-1, "%c", ' ');
		mvwprintw(wnd, row+k, col-2, "%c", ' ');
		mvwprintw(wnd, row+k, col+1, "%c", ' ');
		mvwprintw(wnd, row+k, col+2, "%c", ' ');
		mvwprintw(wnd, row+k, col+3, "%c", ' ');
	}
	mvwprintw(wnd, row-1, col, "%c", ' ');
	mvwprintw(wnd, row+1, col, "%c", ' ');
}

void actualizare_fereastra(WINDOW *wnd, int **a, int *p)
{
	int i, j, row = INIT_ROW+6, col = INIT_COL+3;
	time_t t = time(NULL);
	start_color();
	init_color(COLOR_RED, 100, 0, 0); //modifica coloarea;
	init_pair(1, COLOR_WHITE, COLOR_GREEN); //initializeaza perechi de culori;
	init_pair(2, COLOR_WHITE, COLOR_RED);
	init_pair(3, COLOR_WHITE, COLOR_CYAN);
	init_pair(4, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(5, COLOR_WHITE, COLOR_YELLOW);
	for(i = 0; i < 4; i++)
	{
		row = row+3;
		col = INIT_COL+3;
		for(j = 0; j < 4; j++)
		{
			switch(a[i][j]) //printeaza numar si coloreaza casuta lui;
			{
				case 0:
					wattron(wnd, COLOR_PAIR(3));
					coloreaza(wnd, row, col);
					mvwprintw(wnd, row, col-3, "%4d", a[i][j]);
					wattroff(wnd, COLOR_PAIR(3));
					break;
				case 2:
					wattron(wnd, COLOR_PAIR(4));
					coloreaza(wnd, row, col);
					mvwprintw(wnd, row, col-3, "%4d", a[i][j]);
					wattroff(wnd, COLOR_PAIR(4));
					break;
				case 4:
					wattron(wnd, COLOR_PAIR(1));
					coloreaza(wnd, row, col);
					mvwprintw(wnd, row, col-3, "%4d", a[i][j]);
					wattroff(wnd, COLOR_PAIR(1));
					break;
				case 8:
					wattron(wnd, COLOR_PAIR(2));
					coloreaza(wnd, row, col);
					mvwprintw(wnd, row, col-3, "%4d", a[i][j]);
					wattroff(wnd, COLOR_PAIR(2));
					break;
				default:
					wattron(wnd, COLOR_PAIR(5));
					coloreaza(wnd, row, col);
					mvwprintw(wnd, row, col-3, "%4d", a[i][j]);
					wattroff(wnd, COLOR_PAIR(5));
					break;


			}

			col = col+7;
		}
	}
	mvwprintw(wnd, row+i+4, INIT_COL, "SCOR %d", *p); //afisare scor;
	mvwprintw(wnd, row+i+4, col/2+10, "	%s", ctime(&t)); //afisare timp;
}


void generare_numar(int **a) //genereaza un numar si genereaza doi indici pana la gasirea unei pozitii valide ( egale cu 0);
{
	int random_number, i, j;
	srand(time(NULL));
	random_number = rand () % MAX1;
	random_number = (random_number+1)*2;
	do
	{
		srand(time(NULL));
		i = rand () % MAX2;
		srand(time(NULL));
		j = rand () % MAX2;
		if(a[i][j] == 0)
		{
			a[i][j] = random_number;
			break;
		}
	}
	while(a[i][j] != 0);
}

void deplasare(int **a, int ws, int ad, WINDOW *wnd, int *p)
{
	int i, j, k;
	if(ad == (-1)) //deplasare dreapta;
	j = 3;
	if(ad == 1) //deplasare stanga;
	j = 0;
	if(ws == (-1)) //deplasare jos;
	i = 3;
	if(ws == 1) //deplasare sus;
	i = 0;
	for(k = 0; k < 4; k++)
	{
		if(ws == 0)
			i = k; //se parcurg liniile;
		else
			j = k; //se parcurg coloanele;
		/* se porneste de la capatul (liniei sau coloanei) spre care se face deplasarea si se verifica relatiile dintre "casute",
		   care sunt egale, unde se afla 0, unde se face suma si care este prioritatea de adunare si, respectiv, cum se muta ele;
		   dupa gasirea unui pattern de mutare (rezolvare) a unei linii sau coloane, se iese din while cu break si se continua for-ul
		   pentru urmatoarea linie sau coloana;*/
		/* mentionez ca algoritmul prezentat nu este cel mai eficient, la anumite deplasari, dureaza mai mult timp la efectuarea mutarii,
		   dar programul nu da crash, ci se rezolva, intr-un final, deplasarea;*/
		/* se actualizeaza punctajul dupa fiecare deplasare;*/
		while (FOREVER)
		{
			if(a[i][j] != 0)
			{
				if(a[i+ws][j+ad] != 0)
				{
					if(a[i][j] == a[i+ws][j+ad])
					{
						a[i][j] = a[i][j]*2;
						*p = (*p)+a[i][j];
						if(a[i+ws*2][j+ad*2] != 0 && a[i+ws*3][j+ad*3] == a[i+ws*2][j+ad*2])
						{
							a[i+ws][j+ad] = a[i+ws*2][j+ad*2]*2;
							a[i+ws*2][j+ad*2] = 0;
							a[i+ws*3][j+ad*3] = 0;
							*p = (*p)+ a[i+ws][j+ad];
							break;
						}
						if(a[i+ws*2][j+ad*2] == 0)
						{
							a[i+ws][j+ad] = a[i+ws*3][j+ad*3];
							a[i+ws*3][j+ad*3] = 0;
							*p = (*p)+a[i+ws][j+ad];
							break;
						}
						a[i+ws][j+ad] = a[i+ws*2][j+ad*2];
						a[i+ws*2][j+ad*2] = a[i+ws*3][j+ad*3];
						a[i+ws*3][j+ad*3] = 0;
						break;
					}
					else
					{
						if(a[i+ws*2][j+ad*2] != 0)
						{
							if(a[i+ws*2][j+ad*2] == a[i+ws][j+ad])
							{
								a[i+ws][j+ad] = a[i+ws][j+ad]*2;
								a[i+ws*2][j+ad*2] = a[i+ws*3][j+ad*3];
								a[i+ws*3][j+ad*3] = 0;
								*p = (*p)+a[i+ws][j+ad];
								break;
							}
							else
								if(a[i+ws*3][j+ad*3] == a[i+ws*2][j+ad*2])
								{
									a[i+ws*2][j+ad*2] = a[i+ws*2][j+ad*2]*2;
									a[i+ws*3][j+ad*3] = 0;
									*p = (*p)+a[i+ws*2][j+ad*2];
									break;
								}
							break;
						}
						else
						{
							if(a[i+ws*3][j+ad*3] == a[i+ws][j+ad])
							{
								a[i+ws][j+ad] = a[i+ws][j+ad]*2;
								a[i+ws*3][j+ad*3] = 0;
								*p = (*p)+a[i+ws][j+ad];
								break;
							}
							else
							{
								a[i+ws*2][j+ad*2] = a[i+ws*3][j+ad*3];
								a[i+ws*3][j+ad*3] = 0;
								break;
							}
						}
						break;
					}
				}

				else
				{
					if(a[i+ws*2][j+ad*2] == a[i][j])
					{
						a[i][j] = a[i][j]*2;
						a[i+ws][j+ad] = a[i+ws*3][j+ad*3];
						a[i+ws*2][j+ad*2] = 0;
						a[i+ws*3][j+ad*3] = 0;
						*p = (*p)+a[i][j];
						break;
					}
					else
						if(a[i+ws*2][j+ad*2] == 0)
						{
							if(a[i+ws*3][j+ad*3] == a[i][j])
							{
								a[i][j] = a[i][j]*2;
								a[i+ws*3][j+ad*3] = 0;
								*p = (*p)+a[i][j];
								break;
							}
							else
								if(a[i+ws*3][j+ad*3] != 0)
								{
									a[i+ws][j+ad] = a[i+ws*3][j+ad*3];
									a[i+ws*3][j+ad*3] = 0;
									break;
								}
							break;
						}
						else
							if(a[i+ws*2][j+ad*2] == a[i+ws*3][j+ad*3])
							{
								a[i+ws][j+ad] = a[i+ws*2][j+ad*2]*2;
								a[i+ws*2][j+ad*2] = 0;
								a[i+ws*3][j+ad*3] = 0;
								*p = (*p)+a[i+ws][j+ad];
								break;
							}
					a[i+ws][j+ad] = a[i+ws*2][j+ad*2];
					a[i+ws*2][j+ad*2] = a[i+ws*3][j+ad*3];
					a[i+ws*3][j+ad*3] = 0;
					break;
				}
			}

			else
			{
				if(a[i+ws][j+ad]!=0)
				{
					if(a[i+ws*2][j+ad*2] == a[i+ws][j+ad])
					{
						a[i][j] = a[i+ws*2][j+ad*2]*2;
						a[i+ws][j+ad] = a[i+ws*3][j+ad*3];
						a[i+ws*2][j+ad*2] = 0;
						a[i+ws*3][j+ad*3] = 0;
						*p = (*p)+a[i][j];
						break;
					}
					if(a[i+ws*2][j+ad*2] != 0 && a[i+ws*2][j+ad*2] == a[i+ws*3][j+ad*3])
					{
						a[i][j] = a[i+ws*2][j+ad*2];
						a[i+ws][j+ad] = a[i+ws*3][j+ad*3]*2;
						a[i+ws*2][j+ad*2] = 0;
						a[i+ws*3][j+ad*3] = 0;
						*p = (*p)+a[i+ws][j+ad];
						break;
					}
					if(a[i+ws*2][j+ad*2] == 0 && a[i+ws*3][j+ad*3] != 0)
					{
						if(a[i+ws*3][j+ad*3] == a[i+ws][j+ad])
						{
							a[i][j] = a[i+ws][j+ad]*2;
							a[i+ws][j+ad] = 0;
							a[i+ws*3][j+ad*3] = 0;
							*p = (*p)+a[i][j];
							break;
						}
						else
						{
							a[i][j] = a[i+ws][j+ad];
							a[i+ws][j+ad] = a[i+ws*3][j+ad*3];
							a[i+ws*3][j+ad*3] = 0;
							break;
						}
					}
					a[i][j] = a[i+ws][j+ad];
					a[i+ws][j+ad] = a[i+ws*2][j+ad*2];
					a[i+ws*2][j+ad*2] = a[i+ws*3][j+ad*3];
					a[i+ws*3][j+ad*3] = 0;
					break;
				}
				else
				{
					if(a[i+ws*2][j+ad*2] == a[i+ws*3][j+ad*3])
					{
						a[i][j] = a[i+ws*2][j+ad*2]*2;
						a[i+ws*2][j+ad*2] = 0;
						a[i+ws*3][j+ad*3] = 0;
						*p = (*p)+a[i][j];
						break;
					}
					if(a[i+ws*2][j+ad*2] == 0)
					{
						a[i][j] = a[i+ws*3][j+ad*3];
						a[i+ws*3][j+ad*3] = 0;
						break;
					}
					a[i][j] = a[i+ws*2][j+ad*2];
					a[i+ws][j+ad] = a[i+ws*3][j+ad*3];
					a[i+ws*2][j+ad*2] = 0;
					a[i+ws*3][j+ad*3] = 0;
					break;
				}
			}
		}
	}

}

int matr_zero(int **a) //numara cati de 0 sunt intr-o matrice;
{
	int i, j, nr = 0;
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			if(a[i][j] == 0)
				nr++;

	return nr;
}

void copy_matrix( int **a, int **b) //copiaza doua matrice;
{
	int i, j;
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			b[i][j] = a[i][j];
}


void auto_solve (int **a, int **b, WINDOW *wnd, int *punctaj, int *punctajb) //miscarea automata a celulelor;
{
	int x = 0, max = 0, depl;
	copy_matrix(a, b); //se retine matricea initiala pe care se va aplica deplasarea decisa a fi cea mai buna;

	/* se testeaza, pe rand, care dintre deplasari elibereaza cele mai multe celule de 0, si se retine deplasarea in depl*/

	deplasare(a, 0, 1, wnd, &x);
	if(matr_zero(a) > max)
	{
		max = matr_zero(a);
		depl = 1;
	}

	copy_matrix(b, a);
	deplasare(a, 0, -1, wnd, &x);
	if(matr_zero(a) > max)
	{
		max = matr_zero(a);
		depl = 2;
	}
	copy_matrix(b, a);
	deplasare(a, 1, 0, wnd, &x);
	if(matr_zero(a) > max)
	{
		max = matr_zero(a);
		depl = 3;
	}
	copy_matrix(b, a);
	deplasare(a, -1, 0, wnd, &x);
	if(matr_zero(a) > max)
	{
		max = matr_zero(a);
		depl = 4;
	}
	copy_matrix(b, a); //se pastreaza matricea b, precum, mai jos, si punctajul;
	switch( depl ) //se efectueaza deplasarea decisa;
	{
		case 1:
			*punctajb = *punctaj;
			deplasare(a, 0, 1, wnd, punctaj);
			break;
		case 2:
			*punctajb = *punctaj;
			deplasare(a, 0, -1, wnd, punctaj);
			break;
		case 3:
			*punctajb = *punctaj;
			deplasare(a, 1, 0, wnd, punctaj);
			break;
		case 4:
			*punctajb = *punctaj;
			deplasare(a, -1, 0, wnd, punctaj);
			break;
	}

}

void make_zero(int **a) //egaleaza toate elementele unei matrice cu 0;
{
	int i, j;
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			a[i][j] = 0;
}

int compare_matrix(int **a, int **b) //compara doua matrice;
{
	int i, j;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			if(a[i][j] != b[i][j])
				return 1; //daca sunt diferite;
	return 0; //daca sunt egale;
}

int verify_matrix(int **a) //verifica daca o matrice are toate elementele diferite de 0 (return 1);
{
	int i, j;
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			if(a[i][j] == 0)
				return 0;
	return 1;
}


void joc_pierdut(WINDOW *wnd, int *punctaj) //afiseaza scorul si un efect de blink-ing;
{
	wclear(wnd);
	wattron(wnd, A_BLINK);
	mvwprintw(wnd, INIT_ROW, INIT_COL, "%s", "GAME OVER");
	mvwprintw(wnd, INIT_ROW+1, INIT_COL, "%s", "SCOR:");
	mvwprintw(wnd, INIT_ROW+1, INIT_COL+6, "%d", *punctaj);
	wrefresh(wnd);
	wattroff(wnd, A_BLINK);
}

void new_game(WINDOW *wnd, int **a, int **b, int *punctaj)
{
	int punctajb, played = 0;
	char ch;
	int nfds, sel;
	fd_set read_descriptors;
	struct timeval timeout;
	nfds = 1; //numarul de elemente din multimea read_descriptors;
	FD_ZERO(&read_descriptors); //se curata multimea de lucru pt select;
	FD_SET(KEYBOARD, &read_descriptors); //se adauga tasatatura la multime;
	timeout.tv_sec = S_TO_WAIT;
	timeout.tv_usec = MILIS_TO_WAIT;

	creare_tabel(wnd);
	wrefresh(wnd);
	generare_numar(a);
	actualizare_fereastra(wnd, a, punctaj);
	wrefresh(wnd);

	// ramane in while pana primeste tasta 'q';
	while (FOREVER)
	{
		sel = select(nfds, &read_descriptors, NULL, NULL, &timeout); //asteapta evenimente de la tastatura;
		switch (sel)
		{
			case SELECT_EVENT:	// daca se selecteaza un singur caracter;
				ch = getchar();
				if (tolower(ch) == 'q')
				{
					break;
				}
				switch (tolower(ch)) //se retine matricea a in b, si punctajul si se efectueaza deplasarea indicata de caracterul introdus;
				{
					case 'a':
						copy_matrix(a, b);
						punctajb = *punctaj;
						deplasare(a, 0, 1, wnd, punctaj);
						if(compare_matrix(a, b)) //daca s-a efectuat miscare valida( matricele sunt diferite), se genereaza un nou numar;
						generare_numar(a);
						break;

					case 'd':
						copy_matrix(a, b);
						punctajb = *punctaj;
						deplasare(a, 0, -1, wnd, punctaj);
						if(compare_matrix(a, b))
							generare_numar(a);
						break;

					case 'w':
						copy_matrix(a, b);
						punctajb = *punctaj;
						deplasare(a, 1, 0, wnd, punctaj);
						if(compare_matrix(a, b))
							generare_numar(a);
						break;

					case 's':
						copy_matrix(a, b);
						punctajb = *punctaj;
						deplasare(a, -1, 0, wnd, punctaj);
						if(compare_matrix(a, b))
							generare_numar(a);
						break;
					case 'u':
						copy_matrix(b, a);
						*punctaj = punctajb;
						break;


				}
				break;
			case SELECT_NO_EVENT:
				//daca nu se selecteaza nimic si expira timeout-ul, se porneste miscarea automata a celulelor;
				auto_solve(a, b, wnd, punctaj, &punctajb);
				if(verify_matrix(a) ==1 && compare_matrix(a,b) == 0) //daca nu mai sunt mutari valide, se iese din switch;
				break;
				generare_numar(a);
				break;

		}
		if( tolower(ch) == 'q')
			break;
		actualizare_fereastra(wnd, a, punctaj); //se actualizeaza fereastra, dupa deplasare;
		wrefresh(wnd);
		if(verify_matrix(a) == 1 && compare_matrix(a, b) == 0) //daca nu mai sunt mutari valide, jocul este pierdut;
		{
			played = 1;
			joc_pierdut(wnd, punctaj);
		}
		FD_SET(KEYBOARD, &read_descriptors); //se reinitializeaza descriptorii si timeout-ul;
		timeout.tv_sec = S_TO_WAIT;
		timeout.tv_usec = MILIS_TO_WAIT;

	}
	if(played == 1)
	{
		make_zero(a);
		punctaj = 0;
	}

}


//functie creare meniu;

void meniu(WINDOW *wnd, int **a, int **b, int *punctaj)
{
	char list[3][9] = { "New Game", "Resume", "Quit" };
	char item[9];
	int i, ch, flag = 0, g = 0;

	wclear(wnd);
	box(wnd, 0, 0); //seteaza marginile ferestrei;

	//printeaza meniul cu optiunile sale si marcheaza cu highlight prima optiune;
	for( i=0; i<3; i++)
	{
		if( i==0 )
			wattron( wnd, A_STANDOUT); //marcheaza cu highlight prima optiune;
		else
			wattroff( wnd, A_STANDOUT);
		sprintf( item, "%-9s", list[i]);
		mvwprintw( wnd, i+1, 2, "%s", item);
	}

	wrefresh(wnd); //se face update la ecranul terminalului;
	i=0;
	noecho(); //nu mai apar caracterele printate pe ecran;
	keypad(wnd, TRUE); //input de la tastatura;
	cbreak();
	curs_set(0);//se ascunde cursorul;

	//se citeste input-ul;
	ch = wgetch(wnd);
	while ( ch != 'q')
	{	flag = 0;
		sprintf(item,"%-9s", list[i]);
		mvwprintw( wnd, i+1, 2, "%s", item );
		//foloseste variabila;
		switch ( ch )
		{
			case KEY_UP:
				i--;
				if(i < 0)
					i = 2;
				break;
			case KEY_DOWN:
				i++;
				if(i > 2)
					i = 0;
				break;
			case 10:
				if(strcmp("New Game", list[i]) == 0)
				{
					make_zero(a);
					make_zero(b);
					*punctaj = 0;
					new_game(wnd, a, b, punctaj);
				}
				if(strcmp("Resume", list[i]) == 0)
				{
					if(matr_zero(a) < 16)
						new_game(wnd, a, b, punctaj);

				}
				if(strcmp("Quit", list[i]) == 0)
					flag = 1;
				g = 1;
				break;
		}
		if(g == 1)
			break;
		if(flag == 1)
			break;
		if(ch == 'q')
			break;
		wattron( wnd, A_STANDOUT );
		sprintf(item, "%-9s", list[i]);
		mvwprintw( wnd, i+1, 2, "%s", item);
		wattroff( wnd, A_STANDOUT);
		ch = wgetch(wnd);

	}
	if(g == 1 && flag == 0)
		meniu(wnd, a, b, punctaj);
	endwin();

}


int main ()
{
	WINDOW *w;
	initscr(); //initializare ncurses;
	start_color();
	w = newwin(50, 70, INIT_ROW, INIT_COL); //creeaza o noua fereastra;
	int **a, **b, i, punctaj = 0;
	a = calloc(4, sizeof(int *));
	b = calloc(4, sizeof(int *));
	for(i=0; i<4; i++)
	{
		a[i] = calloc(4, sizeof(int));
		b[i] = calloc(4, sizeof(int));
	}
	meniu(w, a, b, &punctaj);
	endwin();
	for( i=0; i<4; i++)
	{
		free(a[i]);
		free(b[i]);
	}
	free(a);
	free(b);
	return 0;
}
