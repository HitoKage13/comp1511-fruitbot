// fruit_bot.c
// Assignment 3, COMP1511 18s1: Fruit Bot
//
// This program by Jeremy Lim (z5209627) on INSERT-DATE-HERE
//
// Features:
// - Creates a struct that calculates efficiency.
//
// To do list:
// - Relate efficiency to optimal moves
// - Make sure bot doesn't go into a loop
// - Calculates profit for quantity and NOT per unit.
//
// Submitted 31/05 (11:24pm)
// 0.44 average
// Version 1.0.0: Assignment released.

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "fruit_bot.h"

// Defines for checkAction function (used for determining the actions
// for a given turn).
#define MOVE_FRUIT_ANYTHING 0
#define BUY_FRUIT 1
#define SELL_FRUIT 2
#define MOVE_FRUIT 3
#define BUY_ELEC 4
#define MOVE_ELEC 5
#define MOVE_NULL 6

// Defines for checkEnd function (used for determining when to quit).
#define COMPLETE 7
#define NOT_COMPLETE 8

// Defines for checkValidBuyer function (used for determining if
// there is a valid buyer)
#define BUYER 9
#define NO_BUYER 10
#define BUYER_ANYTHING 11

// Defines for checkValidSeller function (used for determining if
// there is a valid seller)
#define SELLER 12
#define NO_SELLER 13

#define WEST 14
#define EAST 15
#define DEST_W 16
#define DEST_E 17

#define ELEC 18
#define NO_ELEC 19
#define NOT_WORTH_ELEC 20

void print_player_name(void);
void print_move(struct bot *b);
void run_unit_tests(void);

// ADD PROTOTYPES FOR YOUR FUNCTIONS HERE

typedef struct _efficiency{
  int profit;
  int turns;
  int distance;
  int journey;
  int direction;
  double efficiency;
  char fruit[MAX_NAME_CHARS + 1];
  char buyLocation[MAX_NAME_CHARS + 1];
  char sellLocation[MAX_NAME_CHARS + 1];
  struct _efficiency *next;
} efficiency;

typedef struct _trading{
  int profit;
  int turns;
  int distance;
  int journey;
  int direction;
  double efficiency;
  char fruit[MAX_NAME_CHARS + 1];
  char buyLocation[MAX_NAME_CHARS + 1];
  char sellLocation[MAX_NAME_CHARS + 1];
  struct _trading *next;
} trading;

void sort_efficiency (struct bot *b, struct _efficiency locations[MAX_LOCATIONS + 1]);
void sort_trade_efficiency (struct bot *b, struct _trading selling[MAX_LOCATIONS + 1]);
void print_efficiency (struct bot *b, struct _efficiency locations[MAX_LOCATIONS + 1], int i);
void print_sell_efficiency (struct bot *b, struct _trading selling[MAX_LOCATIONS + 1], int i);
void print_buy_efficiency (struct bot *b, struct _trading buying[MAX_LOCATIONS + 1], int i);
int closestAnything (struct bot *b, struct location *anythingName);
int botCount (struct bot *b);
int criticalElectricity (struct bot *b);
int rechargeAmount (struct bot *b, int critical, int nearestBuyer);
int checkAction(struct bot *b, int critical, int botCounter,
struct _efficiency locations[MAX_LOCATIONS + 1], struct _trading selling[MAX_LOCATIONS + 1]);
int checkEnd (struct bot *b);
int checkCosts (struct bot *b);
int checkMoves(struct bot *b, int botCounter, struct _efficiency locations[MAX_LOCATIONS + 1],
struct _trading selling[MAX_LOCATIONS + 1]);
int checkValidBuyer (struct bot *b, char selectedFruit[MAX_NAME_CHARS + 1]);
int checkValidSeller (struct bot *b, char selectedFruit[MAX_NAME_CHARS + 1]);
int checkValidElectricity (struct bot *b);
int nearestElectricity(struct bot *b, int critical);
int nearestRoute(struct location *a, struct location *b);

// YOU SHOULD NOT NEED TO CHANGE THIS MAIN FUNCTION

int main(int argc, char *argv[]) {

    if (argc > 2) {
        // supply any command-line argument to run unit tests
        run_unit_tests();
        return 0;
    }

    struct bot *me = fruit_bot_input(stdin);
    if (me == NULL) {
        print_player_name();
    } else {
        print_move(me);
    }

    return 0;
}

void print_player_name(void) {
    // CHANGE THIS PRINTF TO YOUR DESIRED PLAYER NAME
    printf("Limitless");

}

// print_move - should print a single line indicating
//              the move your bot wishes to make
//
// This line should contain only the word Move, Sell or Buy
// followed by a single positive integer

void print_move(struct bot *b) {
    // THE LINES BELOW IMPLEMENT A SIMPLE (USELESS) STRATEGY
    // REPLACE THEN WITH YOUR CODE
    struct _efficiency locations[MAX_LOCATIONS + 1];
    struct _trading selling[MAX_LOCATIONS + 1];
    struct location *current = b->location;
    char selectedFruit[MAX_NAME_CHARS + 1];
    struct location *anythingName = b->location;

    sort_efficiency(b,locations);

    if (b->fruit != NULL) {
        strcpy(selectedFruit,b->fruit);
        if (checkValidBuyer(b,selectedFruit) == BUYER) {
            sort_trade_efficiency(b,selling);
        }
    }

    int botCounter = botCount(b);
    int criticalElec = criticalElectricity(b);
    int chargeDist = nearestElectricity(b,criticalElec);
    int action = checkAction(b,criticalElec,botCounter,locations,selling);
    int cost = checkCosts(b);
    int move = checkMoves(b,botCounter,locations,selling);
    int anything = closestAnything(b,anythingName);
    int recharge = rechargeAmount(b,criticalElec,move);

    int i = 0;
    char fruitName[MAX_FRUIT_TYPES];

    if (action == BUY_FRUIT || action == BUY_ELEC) {
        printf("Buy ");
        if (action == BUY_FRUIT) {
            printf("%d\n",cost);
        } else if (action == BUY_ELEC) {
            printf("%d\n",recharge);
        }
    } else if (action == SELL_FRUIT) {
        printf("Sell %d\n",cost);
    } else if (action == MOVE_FRUIT || action == MOVE_ELEC
    || action == MOVE_FRUIT_ANYTHING) {
        printf("Move ");
        if (action == MOVE_FRUIT) {
            printf("%d\n",move);
        } else if (action == MOVE_ELEC) {
            printf("%d\n",chargeDist);
        } else if (action == MOVE_FRUIT_ANYTHING) {
            printf("%d\n",anything);
        }
    } else if (action == COMPLETE) {
        printf("Move 0\n");
    }

}


// ADD A COMMENT HERE EXPLAINING YOUR OVERALL TESTING STRATEGY

void run_unit_tests(void) {
    // PUT YOUR UNIT TESTS HERE
    // This is a difficult assignment to write unit tests for,
    // but make sure you describe your testing strategy above.
}


// ADD YOUR FUNCTIONS HERE

void sort_efficiency (struct bot *b, struct _efficiency locations[MAX_LOCATIONS + 1]) {
    struct location *prev = b->location;
    struct location *check = prev;
    struct location *temp = prev;
    struct _efficiency sortHolder[MAX_LOCATIONS + 1];
    int i = 0;
    int j = 0;
    int flag = 1;
    int tempFlag = 1;
    int journeyCounter = 0;
    int distanceCounter = 0;
    int route = 0;
    int sortCounter = 0;
    int turnCheck = 2;
    int sellQuantity = 0;
    int buyQuantity = 0;

    while (!(check == prev && flag == 0)) {
        flag = 0;
        if (strcmp(check->fruit,"Electricity") != 0 && (check->price < 0 &&
        check->quantity > 0)) {
            temp = check;
            distanceCounter = 0;
            tempFlag = 1;
            while (!(temp == check && tempFlag == 0)) {
                tempFlag = 0;
                if ((strcmp(check->fruit,temp->fruit) == 0 && temp->price > 0)
                && temp->quantity > 0) {
                    strcpy(locations[i].sellLocation,check->name);
                    if (temp->quantity == 0 && strcmp(temp->fruit,b->fruit) == 0) {
                        strcpy(locations[i].buyLocation,temp->name);
                    } else if (temp->quantity > 0) {
                        strcpy(locations[i].buyLocation,temp->name);
                    }
                    strcpy(locations[i].fruit,check->fruit);
                    if (check->quantity >= b->maximum_fruit_kg) {
                        sellQuantity = b->maximum_fruit_kg;
                    } else if (check->quantity < b->maximum_fruit_kg) {
                        sellQuantity = check->quantity;
                    }
                    if (temp->quantity >= b->maximum_fruit_kg) {
                        buyQuantity = b->maximum_fruit_kg;
                    } else if (temp->quantity < b->maximum_fruit_kg) {
                        buyQuantity = check->quantity;
                    }
                    locations[i].profit = check->price*(sellQuantity)
                    + temp->price*(buyQuantity);
                    locations[i].distance = distanceCounter;
                    locations[i].journey = journeyCounter;
                    if (strcmp(check->name,temp->name) == 0) {
                        locations[i].direction = DEST_E;
                    } else {
                        locations[i].direction = EAST;
                    }
                    if (strcmp(prev->name,locations[i].buyLocation) == 0) {
                        turnCheck--;
                    } else if (locations[i].journey % b->maximum_move > 0) {
                        turnCheck++;
                    }
                    locations[i].turns = turnCheck + (locations[i].journey/b->maximum_move);
                    if (check->quantity > 0) {
                        locations[i].efficiency = (locations[i].profit/locations[i].turns)*10;
                    } else if (check->quantity == 0) {
                        locations[i].efficiency = 0;
                    }
                    if (b->fruit != NULL && strcmp(b->fruit,locations[i].fruit) != 0) {
                        locations[i].efficiency = 0;
                    }
                    turnCheck = 2;
                    i++;
                }

                temp = temp->east;
                distanceCounter++;
            }
        }
        check = check->east;
        journeyCounter++;
    }

    flag = 1;
    tempFlag = 1;
    distanceCounter = 0;
    journeyCounter = 0;

    while (!(check == prev && flag == 0)) {
        flag = 0;
        if (strcmp(check->fruit,"Electricity") != 0 && (check->price < 0)) {
            temp = check;
            distanceCounter = 0;
            tempFlag = 1;
            while (!(temp == check && tempFlag == 0)) {
                tempFlag = 0;
                if ((strcmp(check->fruit,temp->fruit) == 0 && temp->price > 0)
                && temp->quantity > 0) {
                    strcpy(locations[i].sellLocation,check->name);
                    if (temp->quantity == 0 && strcmp(temp->fruit,b->fruit) == 0) {
                        strcpy(locations[i].buyLocation,temp->name);
                    } else if (temp->quantity > 0) {
                        strcpy(locations[i].buyLocation,temp->name);
                    }
                    strcpy(locations[i].fruit,check->fruit);
                    if (check->quantity >= b->maximum_fruit_kg) {
                        sellQuantity = b->maximum_fruit_kg;
                    } else if (check->quantity < b->maximum_fruit_kg) {
                        sellQuantity = check->quantity;
                    }
                    if (temp->quantity >= b->maximum_fruit_kg) {
                        buyQuantity = b->maximum_fruit_kg;
                    } else if (temp->quantity < b->maximum_fruit_kg) {
                        buyQuantity = check->quantity;
                    }
                    locations[i].profit = check->price*(sellQuantity)
                    + temp->price*(buyQuantity);
                    locations[i].distance = distanceCounter;
                    locations[i].journey = journeyCounter;
                    if (strcmp(check->name,temp->name) == 0) {
                        locations[i].direction = DEST_W;
                    } else {
                        locations[i].direction = WEST;
                    }
                    if (strcmp(prev->name,locations[i].buyLocation) == 0) {
                        turnCheck--;
                    } else if (locations[i].journey % b->maximum_move > 0) {
                        turnCheck++;
                    }
                    locations[i].turns = turnCheck + (locations[i].journey/b->maximum_move);
                    if (check->quantity > 0) {
                        locations[i].efficiency = (locations[i].profit/locations[i].turns)*10;
                    } else if (check->quantity == 0) {
                        locations[i].efficiency = 0;
                    }
                    if (b->fruit != NULL && strcmp(b->fruit,locations[i].fruit) != 0) {
                        locations[i].efficiency = 0;
                    }
                    turnCheck = 2;
                    i++;
                }

                temp = temp->west;
                distanceCounter++;
            }
        }
        check = check->west;
        journeyCounter++;
    }

    while (j < i - 1 || sortCounter > 0) {
        if (locations[j].efficiency < locations[j+1].efficiency) {
            sortHolder[0] = locations[j];
            locations[j] = locations[j+1];
            locations[j+1] = sortHolder[0];
            sortCounter++;
        }

        if (j == i - 2) {
            if (sortCounter > 0) {
                sortCounter = 0;
                j = 0;
            } else {
                break;
            }
        } else {
            j++;
        }
    }

    /* print_efficiency(b,locations,i); */
}

void sort_trade_efficiency (struct bot *b, struct _trading selling[MAX_LOCATIONS + 1]) {
    struct location *prev = b->location;
    struct location *check = prev;
    struct location *temp = prev;
    struct _trading sortHolder[MAX_LOCATIONS + 1];
    char selectedFruit[MAX_NAME_CHARS + 1];
    struct location *anythingName = b->location;
    int i = 0;
    int j = 0;
    int flag = 1;
    int tempFlag = 1;
    int journeyCounter = 0;
    int distanceCounter = 0;
    int route = 0;
    int sortCounter = 0;
    int turnCheck = 2;
    int sellQuantity = 0;

    strcpy(selectedFruit,b->fruit);
    if (b->fruit != NULL && checkValidBuyer(b,selectedFruit) == BUYER_ANYTHING) {
        int anything = closestAnything(b,anythingName);
        strcpy(selling[i].fruit,b->fruit);
        strcpy(selling[i].buyLocation,anythingName->name);
        if (anythingName->quantity >= b->maximum_fruit_kg) {
            sellQuantity = b->maximum_fruit_kg;
        } else if (anythingName->quantity < b->maximum_fruit_kg) {
            sellQuantity = check->quantity;
        }
        selling[i].profit = check->price*(sellQuantity);
        route = nearestRoute(prev,anythingName);
        if (route < 0) {
            route = route*(-1);
        }
        selling[i].distance = route;
        if (strcmp(prev->name,selling[i].buyLocation) == 0) {
            turnCheck--;
        } else if (selling[i].distance % b->maximum_move > 0) {
            turnCheck++;
        }
        selling[i].turns = turnCheck + (selling[i].distance/b->maximum_move);
        selling[i].efficiency = (selling[i].profit/selling[i].turns)*1000;
    } else if (b->fruit != NULL && checkValidBuyer(b,selectedFruit) == BUYER) {
        while (!(check == prev && flag == 0)) {
            flag = 0;
            turnCheck = 2;
            if (strcmp(b->fruit,check->fruit) == 0 && (check->price > 0 &&
            check->quantity > 0)) {
                strcpy(selling[i].fruit,check->fruit);
                strcpy(selling[i].buyLocation,check->name);
                if (check->quantity >= b->maximum_fruit_kg) {
                    sellQuantity = b->maximum_fruit_kg;
                } else if (check->quantity < b->maximum_fruit_kg) {
                    sellQuantity = check->quantity;
                }
                selling[i].profit = check->price*(sellQuantity);
                route = nearestRoute(prev,check);
                if (route < 0) {
                    route = route*(-1);
                }
                selling[i].distance = route;
                if (strcmp(prev->name,selling[i].buyLocation) == 0) {
                    turnCheck--;
                } else if (selling[i].distance % b->maximum_move > 0) {
                    turnCheck++;
                }
                selling[i].turns = turnCheck + (selling[i].distance/b->maximum_move);
                selling[i].efficiency = (selling[i].profit/selling[i].turns)*10;
                i++;
            }

            check = check->east;
        }

        flag = 1;

        while (!(check == prev && flag == 0)) {
            flag = 0;
            turnCheck = 2;
            if (strcmp(b->fruit,check->fruit) == 0 && (check->price > 0 &&
            check->quantity > 0)) {
                strcpy(selling[i].fruit,check->fruit);
                strcpy(selling[i].buyLocation,check->name);
                if (check->quantity >= b->maximum_fruit_kg) {
                    sellQuantity = b->maximum_fruit_kg;
                } else if (check->quantity < b->maximum_fruit_kg) {
                    sellQuantity = check->quantity;
                }
                selling[i].profit = check->price*(sellQuantity);
                route = nearestRoute(prev,check);
                if (route < 0) {
                    route = route*(-1);
                }
                selling[i].distance = route;
                if (strcmp(prev->name,selling[i].buyLocation) == 0) {
                    turnCheck--;
                } else if (selling[i].distance % b->maximum_move > 0) {
                    turnCheck++;
                }
                selling[i].turns = turnCheck + (selling[i].distance/b->maximum_move);
                selling[i].efficiency = (selling[i].profit/selling[i].turns)*10;
                i++;
            }

            check = check->west;
        }

        while (j < i - 1 || sortCounter > 0) {
            if (selling[j].efficiency < selling[j+1].efficiency) {
                sortHolder[0] = selling[j];
                selling[j] = selling[j+1];
                selling[j+1] = sortHolder[0];
                sortCounter++;
            }

            if (j == i - 2) {
                if (sortCounter > 0) {
                    sortCounter = 0;
                    j = 0;
                } else {
                    break;
                }
            } else {
                j++;
            }
        }

        /* print_sell_efficiency(b,selling,i); */
    } /* else if (b->fruit == NULL) {
        while (!(check == prev && flag == 0)) {
            flag = 0;
            turnCheck = 2;
            if (strcmp(b->fruit,check->fruit) == 0 && (check->price < 0 &&
            check->quantity > 0)) {
                strcpy(buying[i].fruit,check->fruit);
                strcpy(buying[i].buyLocation,check->name);
                buying[i].profit = check->price;
                route = nearestRoute(prev,check);
                buying[i].distance = route;
                if (strcmp(prev->name,buying[i].buyLocation) == 0) {
                    turnCheck--;
                } else if (buying[i].distance % b->maximum_move > 0) {
                    turnCheck++;
                }
                buying[i].turns = turnCheck + (buying[i].distance/b->maximum_move);
                buying[i].efficiency = (buying[i].profit/buying[i].turns)*100;
                i++;
            }

            check = check->east;
        }

        flag = 1;

        while (!(check == prev && flag == 0)) {
            flag = 0;
            turnCheck = 2;
            if (strcmp(b->fruit,check->fruit) == 0 && (check->price < 0 &&
            check->quantity > 0)) {
                strcpy(buying[i].fruit,check->fruit);
                strcpy(buying[i].buyLocation,check->name);
                buying[i].profit = check->price;
                route = nearestRoute(prev,check);
                buying[i].distance = route;
                if (strcmp(prev->name,buying[i].buyLocation) == 0) {
                    turnCheck--;
                } else if (buying[i].distance % b->maximum_move > 0) {
                    turnCheck++;
                }
                buying[i].turns = turnCheck + (buying[i].distance/b->maximum_move);
                buying[i].efficiency = (buying[i].profit/buying[i].turns)*100;
                i++;
            }

            check = check->west;
        }

        while (j < i - 1 || sortCounter > 0) {
            if (buying[j].efficiency < buying[j+1].efficiency) {
                sortHolder[0] = buying[j];
                buying[j] = buying[j+1];
                buying[j+1] = sortHolder[0];
                sortCounter++;
            }

            if (j == i - 2) {
                if (sortCounter > 0) {
                    sortCounter = 0;
                    j = 0;
                } else {
                    break;
                }
            } else {
                j++;
            }
        }

        print_buy_efficiency(b,buying,i);
    } */
}

void print_efficiency (struct bot *b, struct _efficiency locations[MAX_LOCATIONS + 1], int i) {
    int j = 0;
    /* while (j < i) { */
        printf("Fruit:              %s\n",locations[j].fruit);
        printf("Profit:             %d\n",locations[j].profit);
        printf("Seller:             %s\n",locations[j].sellLocation);
        printf("Buyer:              %s\n",locations[j].buyLocation);
        printf("Turns:              %d\n",locations[j].turns);
        printf("Dist:               %d\n",locations[j].distance);
        printf("Journey:            %d\n",locations[j].journey);
        if (locations[j].direction == WEST) {
        printf("Direction:          West\n");
        }
        else if (locations[j].direction == EAST) {
        printf("Direction:          East\n");
        }
        else if (locations[j].direction == DEST_W) {
        printf("Direction:          Dest-West\n");
        }
        else if (locations[j].direction == DEST_E) {
        printf("Direction:          Dest-East\n");
        }
        printf("Efficiency:         %.1lf\n",locations[j].efficiency);
        printf("\n");

        j++;

        printf("Fruit:              %s\n",locations[j].fruit);
        printf("Profit:             %d\n",locations[j].profit);
        printf("Seller:             %s\n",locations[j].sellLocation);
        printf("Buyer:              %s\n",locations[j].buyLocation);
        printf("Turns:              %d\n",locations[j].turns);
        printf("Dist:               %d\n",locations[j].distance);
        printf("Journey:            %d\n",locations[j].journey);
        if (locations[j].direction == WEST) {
        printf("Direction:          West\n");
        }
        else if (locations[j].direction == EAST) {
        printf("Direction:          East\n");
        }
        else if (locations[j].direction == DEST_W) {
        printf("Direction:          Dest-West\n");
        }
        else if (locations[j].direction == DEST_E) {
        printf("Direction:          Dest-East\n");
        }
        printf("Efficiency:         %.1lf\n",locations[j].efficiency);
        printf("\n");
        /* j++;
    } */
}

void print_sell_efficiency (struct bot *b, struct _trading selling[MAX_LOCATIONS + 1], int i) {
    int j = 0;
    /* while (j < i) { */
        printf("Fruit:              %s\n",selling[j].fruit);
        printf("Profit:             %d\n",selling[j].profit);
        printf("Buyer:              %s\n",selling[j].buyLocation);
        printf("Turns:              %d\n",selling[j].turns);
        printf("Dist:               %d\n",selling[j].distance);
        printf("Efficiency:         %.1lf\n",selling[j].efficiency);
        printf("\n");
        /* j++;
    } */
}

/* void print_buy_efficiency (struct bot *b, struct _trading buying[MAX_LOCATIONS + 1], int i) {
    int j = 0;
    while (j < i) {
        printf("Fruit:              %s\n",buying[j].fruit);
        printf("Profit:             %d\n",buying[j].profit);
        printf("Seller:              %s\n",buying[j].sellLocation);
        printf("Turns:              %d\n",buying[j].turns);
        printf("Dist:               %d\n",buying[j].distance);
        printf("Efficiency:         %.1lf\n",buying[j].efficiency);
        printf("\n");
        j++;
    }
} */

int closestAnything (struct bot *b, struct location *anythingName) {
    struct location *current = b->location;
    struct location *westCheck = current;
    struct location *eastCheck = current;
    int westCounter = 0;
    int eastCounter = 0;

    while (strcmp(westCheck->fruit,"Anything") != 0 && strcmp(eastCheck->fruit,"Anything") != 0) {
        westCheck = westCheck->west;
        eastCheck = eastCheck->east;
        westCounter--;
        eastCounter++;

        if (strcmp(westCheck->fruit,"Anything") == 0) {
            anythingName = westCheck;
            return westCounter;
        } else if (strcmp(eastCheck->fruit,"Anything") == 0) {
            anythingName = eastCheck;
            return eastCounter;
        }
    }

    return 0;
}

int botCount (struct bot *b) {
    struct bot_list *check = b->location->bots;
    int botCounter = 0;

    while (check != NULL) {
        botCounter++;
        check = check->next;
    }

    return botCounter;
}

// Checks when to recharge electricity
int criticalElectricity (struct bot *b) {
    int critical;
    critical = (b->battery_capacity)/3;
    return critical;
}

// Checks how much to recharge
int rechargeAmount (struct bot *b, int critical, int nearestBuyer) {
    int recharge = 0;
    if (strcmp(b->location->fruit,"Electricity") == 0) {
        if (b->fruit != NULL) {
            if (b->turns_left <= 6) {
                recharge = (b->battery_capacity - b->battery_level)/4;
            } else if (b->turns_left <= 15) {
                recharge = (b->battery_capacity - b->battery_level)/2;
            } else {
                recharge = b->battery_capacity - b->battery_level;
            }
        } else if (b->turns_left <= 6) {
            recharge = (b->battery_capacity - b->battery_level)/4;
        } else if (b->turns_left <= 15) {
            recharge = (b->battery_capacity - b->battery_level)/2;
        } else {
            recharge = b->battery_capacity - b->battery_level;
        }
    }

    return recharge;
}

// Checks what action to take
int checkAction (struct bot *b, int critical, int botCounter, struct _efficiency locations[MAX_LOCATIONS + 1],
struct _trading selling[MAX_LOCATIONS + 1]) {
    struct location *check = b->location;
    char selectedFruit[MAX_NAME_CHARS + 1];
    int closestElectricity = nearestElectricity(b,critical);

    if (checkEnd(b) == COMPLETE) {
        return COMPLETE;
    }

    if (b->battery_level <= critical) {
        if (closestElectricity <= 0 && (locations[0].direction == WEST
        || locations[0].direction == DEST_W)) {
            if (checkValidElectricity(b) == ELEC) {
                if (strcmp(check->fruit,"Electricity") == 0) {
                    return BUY_ELEC;
                } else {
                    return MOVE_ELEC;
                }
            }
        } else if (closestElectricity >= 0 && (locations[0].direction == EAST
        || locations[0].direction == DEST_E)) {
            if (checkValidElectricity(b) == ELEC) {
                if (strcmp(check->fruit,"Electricity") == 0) {
                    return BUY_ELEC;
                } else {
                    return MOVE_ELEC;
                }
            }
        } else if (closestElectricity <= 2*(b->maximum_move) ||
        closestElectricity*(-1) <= 2*(b->maximum_move)) {
            if (checkValidElectricity(b) == ELEC) {
                if (strcmp(check->fruit,"Electricity") == 0) {
                    return BUY_ELEC;
                } else {
                    return MOVE_ELEC;
                }
            }
        }
    }

    if (b->fruit != NULL) {
        strcpy(selectedFruit,b->fruit);
        if (strcmp("Anything",check->fruit) == 0 &&
        checkValidBuyer(b,selectedFruit) == BUYER_ANYTHING) {
            return SELL_FRUIT;
        } else if (strcmp("Anything",check->fruit) != 0 &&
        checkValidBuyer(b,selectedFruit) == BUYER_ANYTHING) {
            return MOVE_FRUIT_ANYTHING;
        } else if (strcmp(check->name,selling[0].buyLocation) == 0 ||
        strcmp(check->fruit,selling[0].fruit) == 0) {
            if ((check->quantity/botCounter) > 0
            && (check->price > 0 && check->quantity > 0)) {
                return SELL_FRUIT;
            }
        }
    } else if (b->fruit == NULL) {
        if (strcmp(check->name,locations[0].sellLocation) == 0 ||
        strcmp(check->fruit,locations[0].fruit) == 0) {
            if ((check->quantity/botCounter) > 0
            && (check->price < 0 && check->quantity > 0)) {
                return BUY_FRUIT;
            }
        }
    }

    return MOVE_FRUIT;

}

// Checks when to stop trading
int checkEnd (struct bot *b) {
    struct location *prev = b->location;
    struct location *check = prev;
    int flag = 1;

    if (b->turns_left <= 2 && b->fruit == NULL) {
        return COMPLETE;
    }

    if (b->battery_level == 0 && b->fruit == NULL) {
        return COMPLETE;
    }

    while (!(check == prev && flag == 0)) {
        flag = 0;

        if (b->fruit != NULL) {
            if (strcmp(b->fruit,check->fruit) == 0 && (check->price > 0
            && check->quantity > 0)) {
                return NOT_COMPLETE;
            }
        } else if (b->fruit == NULL) {
            if ((check->price < 0 && check->quantity > 0)
            && strcmp(check->fruit,"Electricity") != 0) {
                return NOT_COMPLETE;
            }
        }

        check = check->east;
    }

    flag = 1;

    while (!(check == prev && flag == 0)) {
        flag = 0;

        if (b->fruit != NULL) {
            if (strcmp(check->fruit,"Anything") == 0 && (check->price > 0
            && check->quantity > 0)) {
                return NOT_COMPLETE;
            }
        } else if (b->fruit == NULL) {
            if ((check->price < 0 && check->quantity > 0)
            && strcmp(check->fruit,"Electricity") != 0) {
                return NOT_COMPLETE;
            }
        }

        check = check->east;
    }

    return COMPLETE;
}

// Checks the costs and hence how much to buy
int checkCosts (struct bot *b) {
    int cost;
    struct location *check = b->location;

    if (check->quantity == 0) {
        return 0;
    } else if (b->fruit_kg == 0 && check->price < 0) {
        if (check->quantity >= b->maximum_fruit_kg) {
            cost = b->maximum_fruit_kg;
            return cost;
        } else {
            cost = check->quantity;
            return cost;
        }
    } else if (b->fruit_kg != 0 && check->price < 0) {
        if (check->quantity >= (b->maximum_fruit_kg - b->fruit_kg)) {
            cost = (b->maximum_fruit_kg - b->fruit_kg);
            return cost;
        } else {
            cost = check->quantity;
            return cost;
        }
    } else if (b->fruit_kg != 0 && check->price > 0) {
        if (check->quantity >= b->fruit_kg) {
            cost = b->fruit_kg;
            return cost;
        } else {
            cost = check->quantity;
            return cost;
        }
    } else {
        return 0;
    }
}

// Checks how many moves to take.
int checkMoves(struct bot *b, int botCounter, struct _efficiency locations[MAX_LOCATIONS + 1],
    struct _trading selling[MAX_LOCATIONS + 1]) {
    struct location *current = b->location;
    struct location *westCheck = current;
    struct location *eastCheck = current;
    int flag = 1;
    int westCounter = 0;
    int eastCounter = 0;
    int closest = 0;
    char selectedFruit[MAX_NAME_CHARS + 1];
    int move = 0;
    struct location *anythingName = b->location;

    struct location *buyDestination = current;
    struct location *sellDestination = current;

    if (b->fruit != NULL) {
        strcpy(selectedFruit,b->fruit);
        if (checkValidBuyer(b,selectedFruit) == BUYER_ANYTHING) {
            move = closestAnything(b,anythingName);
        } else {
            while (strcmp(buyDestination->name,selling[0].buyLocation) != 0) {
                buyDestination = buyDestination->east;
            }
            move = nearestRoute(current,buyDestination);
        }
        return move;
    } else if (b->fruit == NULL) {
        while (strcmp(sellDestination->name,locations[0].sellLocation) != 0) {
            sellDestination = sellDestination->east;
        } if (locations[0].journey > locations[1].journey) {
            while (strcmp(sellDestination->name,locations[1].sellLocation) != 0) {
                sellDestination = sellDestination->east;
            }
        }
        move = nearestRoute(current,sellDestination);
        return move;
    }

    return move;

    /* while (!(eastCheck == current && flag == 0)) {
        flag = 0;
        if (b->fruit != NULL) {
            if (strcmp(b->fruit,eastCheck->fruit) == 0
            && eastCheck->price > 0) {
                if (eastCheck->quantity > 0) {
                    strcpy(selectedFruit,b->fruit);
                    if (checkValidBuyer(b,selectedFruit) == BUYER) {
                        break;
                    } else if (checkValidBuyer(b,selectedFruit) == BUYER_ANYTHING) {
                        break;
                    }
                }
            } else if (strcmp("Anything",eastCheck->fruit) == 0) {
                strcpy(selectedFruit,b->fruit);
                if (checkValidBuyer(b,selectedFruit) == BUYER_ANYTHING) {
                    break;
                }
            }
        } else if (b->fruit == NULL) {
            if (strcmp(eastCheck->name,locations[0].sellLocation) == 0) {
                if (eastCheck->quantity > 0) {
                    strcpy(selectedFruit,eastCheck->fruit);
                    if (checkValidBuyer(b,selectedFruit) == BUYER) {
                        break;
                    }
                }
            }
        }

        eastCheck = eastCheck->east;
        eastCounter++;
    }

    flag = 1;

    while (!(westCheck == current && flag == 0)) {
        flag = 0;
        if (b->fruit != NULL) {
            if (strcmp(b->fruit,westCheck->fruit) == 0
            && westCheck->price > 0) {
                if (westCheck->quantity > 0) {
                    strcpy(selectedFruit,b->fruit);
                    if (checkValidBuyer(b,selectedFruit) == BUYER) {
                        break;
                    } else if (checkValidBuyer(b,selectedFruit) == BUYER_ANYTHING) {
                        break;
                    }
                }
            } else if (strcmp("Anything",westCheck->fruit) == 0) {
                strcpy(selectedFruit,b->fruit);
                if (checkValidBuyer(b,selectedFruit) == BUYER_ANYTHING) {
                    break;
                }
            }
        } else if (b->fruit == NULL) {
            if (strcmp(westCheck->fruit,locations[0].sellLocation) == 0) {
                if (westCheck->quantity > 0) {
                    strcpy(selectedFruit,westCheck->fruit);
                    if (checkValidSeller(b,selectedFruit) == SELLER) {
                        break;
                    }
                }
            }
        }

        westCheck = westCheck->west;
        westCounter++;
    }

    if (westCounter == 0 && eastCounter != 0) {
        closest = eastCounter;
    } else if (westCounter != 0 && eastCounter == 0) {
        closest = westCounter*(-1);
    } else if (westCounter != 0 && eastCounter != 0) {
        if (westCounter < eastCounter) {
            closest = westCounter*(-1);
        } else if (westCounter > eastCounter) {
            closest = eastCounter;
        } else if (westCounter == eastCounter) {
            closest = eastCounter;
        }
    }

    return closest; */
}

// Checks if there is a valid buyer for a given fruit.
int checkValidBuyer (struct bot *b, char selectedFruit[MAX_NAME_CHARS + 1]) {
    struct location *prev = b->location;
    struct location *check = prev;
    int flag = 1;

    // Checks for buyer of same fruit.
    while (!(check == prev && flag == 0)) {
        flag = 0;

        if (check->fruit != NULL) {
            if (strcmp(check->fruit,selectedFruit) == 0) {
                if (check->price > 0 && check->quantity > 0) {
                    return BUYER;
                }
            }
        }

        check = check->east;
    }

    flag = 1;

    while (!(check == prev && flag == 0)) {
        flag = 0;

        if (check->fruit != NULL) {
            if (strcmp(check->fruit,"Anything") == 0) {
                if (check->price > 0 && check->quantity > 0) {
                    return BUYER_ANYTHING;
                }
            }
        }

        check = check->east;
    }

    return NO_BUYER;
}

// Checks if there is a valid seller for a given fruit.
int checkValidSeller (struct bot *b, char selectedFruit[MAX_NAME_CHARS + 1]) {
    struct location *prev = b->location;
    struct location *check = prev;
    int flag = 1;

    // Checks for seller of same fruit.
    while (!(check == prev && flag == 0)) {
        flag = 0;

        if (check->fruit != NULL) {
            if (strcmp(check->fruit,selectedFruit) == 0) {
                if (check->price < 0 && check->quantity > 0) {
                    return SELLER;
                }
            }
        }

        check = check->east;
    }

    return NO_SELLER;
}

int checkValidElectricity (struct bot *b) {
    struct location *current = b->location;
    struct location *check = current;
    int flag = 1;
    int critical = (b->battery_capacity)/3;

    while (!(current == check && flag == 0)) {
        flag = 0;

        if (strcmp(check->fruit, "Electricity") == 0) {
            if (check->quantity > critical) {
                return ELEC;
            }
        }

        check = check->east;
    }

    flag = 1;

    while (!(current == check && flag == 0)) {
        flag = 0;

        if (strcmp(check->fruit, "Electricity") == 0) {
            if (check->quantity > 0) {
                return NOT_WORTH_ELEC;
            }
        }

        check = check->east;
    }

    return NO_ELEC;
}
// Checks where the nearest electricity location is
int nearestElectricity(struct bot *b, int critical) {
    // PUT YOUR CODE HERE
    struct location *current = b->location;
    struct location *westCheck = current;
    struct location *eastCheck = current;
    int flag = 1;
    int checkFlag = 0;
    int closest = 0;
    int westElec;
    int eastElec;
    int moveWest = 0;
    int moveEast = 0;

    if (strcmp(current->fruit, "Electricity") == 0) {
        closest = 0;
        checkFlag = 1;
    }

    while (!(current == eastCheck && flag == 0)) {
        flag = 0;

        if (strcmp(eastCheck->fruit, "Electricity") == 0) {
            if (eastCheck->quantity > critical) {
                break;
            }
        }

        eastCheck = eastCheck->east;
        moveEast++;
    }

    flag = 1;

    while (!(current == westCheck && flag == 0)) {
        flag = 0;

        if (strcmp(westCheck->fruit, "Electricity") == 0) {
            if (westCheck->quantity > critical) {
                break;
            }
        }

        westCheck = westCheck->west;
        moveWest++;
    }

    westElec = moveWest;
    eastElec = moveEast;

    if (checkFlag == 0) {
        if (westElec == 0 && eastElec != 0) {
            closest = eastElec;
        }

        else if (westElec != 0 && eastElec == 0) {
            closest = westElec*(-1);
        }

        else if (westElec != 0 && westElec != 0) {
            if (westElec < eastElec) {
                closest = westElec*(-1);
            }

            else if (westElec > eastElec) {
                closest = eastElec;
            }

            else if (westElec == eastElec) {
                closest = eastElec;
            }
        }
    }

    return closest;
}

int nearestRoute(struct location *a, struct location *b) {
    // PUT YOUR CODE HERE
    struct location *current = a;
    struct location *destination = b;
    struct location *westCheck = current;
    struct location *eastCheck = current;
    int flag = 1;
    int checkFlag = 0;
    int closest = 0;
    int westCounter = 0;
    int eastCounter = 0;
    int moveWest = 0;
    int moveEast = 0;

    if (strcmp(current->name,destination->name) == 0) {
        closest = 0;
        checkFlag = 1;
    }

    while (!(current == eastCheck && flag == 0)) {
        flag = 0;

        if (strcmp(eastCheck->name, destination->name) == 0) {
            break;
        }

        eastCheck = eastCheck->east;
        moveEast++;
    }

    flag = 1;

    while (!(current == westCheck && flag == 0)) {
        flag = 0;

        if (strcmp(westCheck->name, destination->name) == 0) {
            break;
        }

        westCheck = westCheck->west;
        moveWest++;
    }

    westCounter = moveWest;
    eastCounter = moveEast;

    if (checkFlag == 0) {
        if (westCounter == 0 && eastCounter != 0) {
            closest = eastCounter;
        }

        else if (westCounter != 0 && eastCounter == 0) {
            closest = westCounter*(-1);
        }

        else if (westCounter != 0 && westCounter != 0) {
            if (westCounter < eastCounter) {
                closest = westCounter*(-1);
            }

            else if (westCounter > eastCounter) {
                closest = eastCounter;
            }

            else if (westCounter == eastCounter) {
                closest = eastCounter;
            }
        }
    }

    return closest;
}
