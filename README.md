Pentru a rula: -trebuie instalata libraria ncurses folosind comanda:
               -sudo apt update
               -sudo apt install build-essential libncurses5-dev libncursesw5-dev
               -dupa instalare se ruleaza in compilator folosind comanda:
               -gcc -o snake snake.c -lncurses -lpthread
               -dupa ce compileaza si nu apar erori:
               -./snake 
