#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

// Maximum size of the queue.
#ifndef QUEUE_LIMIT
#define QUEUE_LIMIT 256
#endif

// Maximum number of tasks per thread.
#ifndef TASKS_PER_THREAD
#define TASKS_PER_THREAD 16
#endif

// Structure to hold task information.
typedef struct {
  // Socket file descriptor.
  int socket_fd;
  // IP address.
  char address[16];
} Task;

// Queue structure for holding tasks.
typedef struct {
  Task tasks[QUEUE_LIMIT];
  // Current queue size.
  int size;
  // Front and rear of the queue.
  int front;
  int rear;

  // Used to signal that the queue is not full or not empty.
  pthread_cond_t not_empty;
  pthread_cond_t not_full;

  // Used to indicate that no more tasks will be added to the queue.
  // Set to 1 in the main thread when all tasks are sent.
  int done;
  pthread_mutex_t mutex;
} Queue;

Queue* new_queue(void);
void submit_task(Queue* queue, const char* ip);
void signal_done(Queue* queue);
void free_queue(Queue* queue);

// ************#*#******####*########***#####################%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%@@@@%%%@@@@@@@@@@@@@@@@@@@
// ************************************##################%##%%%@@@@@@@@@@@@@@@@@@@@@@@@@%%#@@@%#**+++++**#%@@@@%%@@@@%%%@@@@%@@@@@@@@@@@@@@
// **++++++**************************#####%%%%%%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+:.:.:..              :-+##%%%@#*#%@@@%@@@@@@@@@@@@@@
// ++==++++******######%%%%%%%%%%%%%%%%%%%###########%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@%-    ...          .        .=*-... .+@@%@@@@@@@@@@@@@@
// ####%%%%%####*****++++++++++***########%%%%%%%%%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@#:   .:...          ....        .      .*%@@@@@@@@@@@@@@
// +++++++*****###########%##################%%%%%%%%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@=    -..      :        .:.                *@@@@@@@@@@@@@@
// ..::......::..::--=+***********###################%%%%%%@@@@@@@@@@@@@@@@@@@@@%:   .=.       .::        .::               .#%%%%%%@@@@@@@
//             ...:::.:=++++**######*#***######%%%%%%%%%%%@@@@@@@@@@@@@@@@@@@@@%.    :      .... :          =                =###%%%%%%%%%%
// :::-::::--------==*#%%%#*******########%%####%%%%%%%%%%@@@@@@@@@@@@@@@@@@@@%.    :      ..:.   :..        :                +***#%%%@@@@@
// +--::::::::::::--=#%@#***###***####*######%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@-    ..    .::.      :.:       :          .     =%@@@@@@@@@@@
// @%+:::::::::::::-**#@##*#####%%%%%@@@@@%%%%%%%%%%%%%%%@@@@@@@@@@@@@@@@@@@@%     :   :--:         :.:.      :         ..    .#%@@@@@@@@@@
// %@@@#=-::--::.:-=**#@###%%#########**######%%%%%%%%%@@@@@@@@@@@@@@@@@@@@@@=    ...:+=:.:.        ::::::    :          .     *%@@@@@@@@@@
// %%@@@@@%+---:.-#=***%########*#*##*######%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@:    -::.               ..:::..  :        : .     +@@@@@@@@@@@
// ##*#%@@@@%+-::*%+***%##**######%%%%%%%%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@     :    ...::        .:... ..::-        .:.   . -%@@@@@@@@@@
// +*****#%@%++***%+**#@##*%%%%%%%%%%%%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@%----+     .  .:--.          .         :       .:-.   . :%@@@@@@@@@@
// *********#@++*#%+**#@##****#######%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%=..+ .   :.-+%##*-.        :=*##*=+:.:       ...  ... .=*#%@@@@@@@
// **********##++===**#@##+***####%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%=..-   +.                  -:::.:-:-       :    : :..::.=@@@@@@@
// **********#%#%%**=##@***#####%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#=-. .+-        ..                :    . .    .    .=#@@@@@@@@@
// **********#%#%#*++##@**#%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@-==..*-.       ::               :::  .=:....   .   *@@@@@@@@@@
// *********#%+%@@++:##@###%%%%@@@@%%##*%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ .::.+.                        .-:..:-++.....      #@@@@@@@@@@
// ********#%+#@@*++-+#@#**#####%%%%@@@@+@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%%%%%%* :..#==       .                  -..=-=:-          #@@@@@@@@@@
// *****##%**%%*+++++++=#%@@@@@@@@@@@@@@#*@%%%%%%%%%%@@@@@@@@@@@@@@%%%%%%%%%- : -*.-:.      ..........         .#+.#:.          =@@@@@@%%%#
// ****######%*#*+++++*+#@@@@@@@@@@@@@@@@+%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%* .: -*.-.-:                      .+@@=-*-           :%########*
// %@@@@@@@@%%#@@@*-#@@**@@@@@@@@@@@@@@@@@+%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#. :. .: - :.--.                 .=%%@@:+*:           .**********
// @@@@@@%%@@@@@@%#-#*+:  -%@@@@@@@@@@@@@@*#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#=  :     ...:.+#+-.            .  +%%%#.-=            .=+*****#**
// @@@@@*  +%%%%%%%***::-. :%@@@@@@@@@@@@@@+%############################*   ..     : :.-#***+-.......      =***=..             -:*********
// @@@@@@+:+*##%%%%%%#####*.-@@@@@@@@@@@@@@%+%#################%%########-    :     :  :.*##**=             :++*:.              = ++++**##%
// @@@@+:-**=---+##*%%#####:.@@@@@@@@@@@@@@@**%%%%%%#####################.    :.     :  -=###*+              :+=.               + =+*###%%%
// @@@@+: =###****+-#+-=-+#- @@@@@@@@@@@@@@@@+%##########################:.    :     ..  +=--:-               :                .+ =*###%##%
// @@@@%**#####***#*%***###= ::==+*##%%%%%%@@#+#######**********##**+==:. .::  .:     :  .:   :              .                 -= -*****###
// @@@@-  +*#*****#*#****##+                        .:=+**#+=-:.            .:  :.     :  :   .             :                 :-  -==++++**
// @@@@%+=*##****#####***##*                                                  :  :     :  :                ..                .  .---=======
// @@@@#:-###**######*#**###                                                   :  :    :  -....    . ......:                .:------=======
// @@@@@@#*#######%%%#*##%#%-:..                                               .: ..   .. :       .       :       .          ---------=-=--
// @@@@@@*##%%###%########**@@@@@%#*+=:.                                  .:::. :  :    : :              ..      :           :-----=-------
// @@@@@%*###*##########**--%@@@@@@@@@@@@%#*+=-:.                 .:--=+++++=+-:.: ..   : :              :   .  ..           :--------=----
// @@@@@%#################+#@@@@@@@@@@@@@@@@@@@@@@***++=========++++++++++++++++.+  :   : :             ...     :  .         :--------:----
// @@@@@@@@%%%@@@@@@@@@@%%@@@@@@@@@@@@@@@@@@@@@@@@@+**********+++++++++++++++=   -  :   : :             : : :  :. .          ...:::....::::
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%****++++++********++++++++...:. -   :.=             :.- :  : ..                      ..
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@***************++*******++    : +  .::  ............:--..  : :                         
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+#***********+++++++++++-    :.-  - :....   .......-:::  ....                         
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#****##******++****++===-    -:: .*.-              :--:  : :                          
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#*+==***++=+***###*+++++++++:   -::.--.=              ::-:  . :                          
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#*++====+*##+:    .=+*+****+**++++=   =  ::.-:              ::::  : :                          
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%*++=-=+++#%%@@@@@%.         :-=********=      :- : .             .=::. -.-                          
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@%#*+====++*#%%@@@@@@@@@@=             .---+**+-       :   .              =.:: :::                          
// @@@@@@@@@@@@@@@@@@@@@@@%#*+====++*#%%@@@@@@@@@@@@@@@#.                     .            :               :- :.=                         :
// @@@@@@@@@@@@@@@@@%#**+===++*#%%@@@@@@@@@@@@@@@@@@@@@:.                    .              :               =:. +.                     :==-
// @@@@@@@@@@@@%**+====+*#%%@@@@@@@@@@@@@@@@@@@@@@@@@@#=*=::                 .               .              ::  -                  ..:----=
// @@@@@@%**+=-==+*#%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%##%%**+*#-               :               .              : . -               -=+=--=====
// %#*+=-=+++*%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%@#+=%%#%%%%%%#:              ...              .               . :           . -+===========

#endif // QUEUE_H
