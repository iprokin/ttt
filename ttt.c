#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define m 3
#define n 3
#define k 3

#define file_path_max_length 1000

#define MAX 362880// * 128// * 2048

typedef int Board[m][n];

typedef struct nd {
   struct nd *parent;
   Board state; 
   int token; // 1 = x, -1 = o
   int num_childs;
   int win;
   //int score;
   struct nd *childs;
} Node;

typedef struct {
    int num_nodes;
    Node **nodes;
} Nodes;

Nodes terminals;

void
init_arr(int rows, int cols, int arr[rows][cols], int val) {
    int i, j;
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
            arr[i][j] = val;
        }
    }
}

int
check_win_sum(int sum) {
    return sum/k;
}

void
check_and_add_to_sum(int *sum, int i, int p, int val) {
   if (i < p) {
       (*sum) += val;
   }
}

int
whowin(int arr[m][n]) {
    int sum_h;
    int sum_v[k];
    int sum_dupdown;
    int sum_ddownup;
    int c;
    int i, j;

    c = 0;
    sum_dupdown = 0;
    sum_ddownup = 0;

    for(i = 0; i < k; i++) {
        sum_v[i] = 0;
    }

    for(i = 0; i < m; i++) {
        sum_h = 0;
        for(j = 0; j < n; j++) {
            check_and_add_to_sum(&sum_h, i, k, arr[i][j]);
            check_and_add_to_sum(&sum_v[j], j, k, arr[i][j]);
            if (i == j) {
                check_and_add_to_sum(&sum_dupdown, i, k, arr[i][j]);
            }
            if ((m-1-i) == j) {
                check_and_add_to_sum(&sum_ddownup, i, k, arr[i][j]);
            }
        }
        c = check_win_sum(sum_h);
        if (c != 0) {
            goto end;
        }
    }
    c = check_win_sum(sum_dupdown);
    if (c != 0) {
        goto end;
    }
    c = check_win_sum(sum_ddownup);
    if (c != 0) {
        goto end;
    }
    for(i = 0; i < k; i++) {
        c = check_win_sum(sum_v[i]);
        if (c != 0) {
            goto end;
        }
    }
    end:
    return c;
}

void
goto_childs(Node *node) {
    int i, j, ic, q;
    if (node->num_childs > 0 && node->win == 0) {
        node->childs = malloc(sizeof(Node) * node->num_childs);
        q = 0;
        for (ic = 0; ic < m*n; ic++) {
            i = ic / n;
            j = ic % n;
            if (node->state[i][j] == 0) {
                node->childs[q].parent = node;
                memcpy(node->childs[q].state, node->state, m*n*sizeof(int));
                node->childs[q].state[i][j] = node->token;
                node->childs[q].token = -node->token; // players change turns
                node->childs[q].num_childs = node->num_childs-1;

                node->childs[q].win = whowin(node->childs[q].state);
                goto_childs(&node->childs[q]);
                q++;
            }
        }
    } else {
        // terminal
        node->num_childs = 0;
        node->childs = NULL;
        terminals.nodes[terminals.num_nodes] = node;
        terminals.num_nodes++;
        assert( terminals.num_nodes <= MAX );
    }
}

void
print_2d_arr(int rows, int cols, int arr[rows][cols]) {
    int i, j;
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols-1; j++)
            printf("%d\t", arr[i][j]);
        printf("%d", arr[i][cols-1]);
        printf("\n");
    }
}

void
save_2d_arr(int rows, int cols, int arr[rows][cols], char *filepath) {
    FILE *fp;
    int i, j;
    fp = fopen(filepath, "w");
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols-1; j++)
            fprintf(fp, "%d\t", arr[i][j]);
        fprintf(fp, "%d", arr[i][cols-1]);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void
read_2d_arr_from_file(int rows, int cols, int arr[rows][cols], char *filepath) {
    FILE *fp;
    int i, j;
    fp = fopen(filepath, "r");
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
            fscanf(fp, "%i", &arr[i][j]);
        }
    }
    fclose(fp);
}

Node*
search_pos(Node *node, Board state_target) {
    int i, j, d, all_equal;
    Node *fnd;
    all_equal = 1;
    fnd = NULL;
    for(i = 0; i < m; i++) {
        for(j = 0; j < n; j++) {
            d = node->state[i][j]-state_target[i][j];
            if (abs(d) == 2) {
                //goto endsearchpos;
                return fnd;
            } else if (d != 0) {
                if (node->state[i][j] == 0) {
                    all_equal = 0;
                } else {
                    //goto endsearchpos;
                    return fnd;
                }
            };
        }
    }
    if (all_equal) {
        fnd = node;
        //goto endsearchpos;
        return fnd;
    } else {
        if (node->num_childs > 0) {
            for(i = 0; i < node->num_childs; i++) {
                fnd = search_pos(&node->childs[i], state_target);
                if (fnd != NULL) {
                    //goto endsearchpos;
                    return fnd;
                }
            }
        }
    }
    //endsearchpos:
    return fnd;
}

int
idofwinnerchild(Node *node) {
    int winner; // winner is max if token=1, min if token=-1
    int i, id_winner;
    winner = node->token*(-MAX+1);
    
    id_winner = -1; // if not changed in the loop it will produce error later
    for(i = 0; i < node->num_childs; i++) {
        if (node->token * node->childs[i].win > node->token * winner) {
            id_winner = i;
            winner = node->childs[i].win;
        }
    }
    assert( id_winner != -1);
    return id_winner;
}

int
go_down(Node *node) {
    int id_winner;
    if (node->num_childs > 0) {
        id_winner = idofwinnerchild(node);
        return go_down(&node->childs[id_winner]);
    } else {
        return node->win;
    }
}

void
go_up_bf(Nodes lastnodes) {
    int idwin;
    int i;
    Nodes layerupnodes;
    Nodes nodes;
    Node* node;

    layerupnodes.nodes = malloc(sizeof(Node*)*lastnodes.num_nodes);

    nodes.num_nodes = lastnodes.num_nodes;
    nodes.nodes = lastnodes.nodes;
    for(;;) {
        layerupnodes.num_nodes = 0;
        for(i = 0; i < nodes.num_nodes; i++) {
            node = nodes.nodes[i];
            if (node->parent != NULL) {
                layerupnodes.nodes[layerupnodes.num_nodes] = node->parent;
                layerupnodes.num_nodes++;
                idwin = idofwinnerchild(node->parent);
                node->parent->win = node->parent->childs[idwin].win;
            }
        }
        if (layerupnodes.num_nodes == 0) {
            break;
        } else {
            nodes.num_nodes = layerupnodes.num_nodes;
            nodes.nodes = layerupnodes.nodes;
        }
    }
}

char convert_to_XO(int val){
    if (val == -1) {
        return 'O';
    } else if (val == 1) {
        return 'X';
    } else {
        return ' ';
    }
}

void
print_line(const int l) {
    int j;
    printf("\n  ");
    for(j = 0; j < l; j++) {
        printf("----");
    }
    printf("-\n");
}

void
print_state(Board state) {
    int i, j;
    printf(" ");
    for(j = 1; j < n+1; j++) {
        printf("   %i", j);
    }
    printf("  ");

    print_line(n);
    for(i = 0; i < m; i++) {
        printf("%c | ", 'a'+i);
        for(j = 0; j < n-1; j++) {
            printf("%c | ", convert_to_XO(state[i][j]));
        }   
        printf("%c |", convert_to_XO(state[i][n-1]));
        print_line(n);
    }
}

void
print_help() {
    printf("\n---------\nTo control the game type:\n---------\n");
    printf("%s\n", ":h print this help\n:q to exit\n:l to load the state of the board\n:s to save the state\na1 to play left upper corner, b2 center.");
}

int
print_state_and_who_wins(Node *fndnode) {
    char c;
    if (fndnode != NULL) {
        print_state(fndnode->state);
        if (fndnode->num_childs == 0) {
            printf("Gave over!\n");
            c = convert_to_XO(fndnode->win);
            if (c == ' ') {
                printf("No one won\n");
            } else {
                printf("%c won\n", c);
            }
            return 0;
        }
        c = convert_to_XO(go_down(fndnode));
        if (c != ' '){
            printf("%c will win\n", c);
        } else {
            printf("No one will win\n");
        }
        return 1;
    } else {
        printf("State is not in game tree!");
        return 0;
    }
}

Node*
read_input_change_state(Node *root, Node *fndnode) {
    int i, j;
    char move[2];
    char filepath[file_path_max_length];
    Node *retfndnode;

    scanf("%s", move);
    Board fndstate;
    retfndnode = fndnode;
    if (strcmp(move, ":q") == 0) {
        exit(1);
    } else if (strcmp(move, ":h") == 0) {
        print_help();
        return read_input_change_state(root, retfndnode);
    } else if (strcmp(move, ":l") == 0) {
        printf("Enter path to file to load:\n");
        scanf("%s", filepath);
        read_2d_arr_from_file(m, n, fndstate, filepath);
        retfndnode = search_pos(root, fndstate);
        // print_2d_arr(m, n, root.state);
    } else if (strcmp(move, ":s") == 0) {
        printf("Enter path to file to save:\n");
        scanf("%s", filepath);
        save_2d_arr(m, n, fndstate, filepath);
    } else {
        i = move[0]-'a';
        j = atoi(&move[1])-1;
        if (i < 0 || i > m || j < 0 || j > n) {
            print_help();
        } else if (fndnode->state[i][j] != 0) {
            printf("This move is unavailable!\n");
            return read_input_change_state(root, retfndnode);
        } else {
            memcpy(fndstate, fndnode->state, m*n*sizeof(int));
            fndstate[i][j] = fndnode->token;
            retfndnode = search_pos(fndnode, fndstate);
        }
    }
    return retfndnode;
}

int
main() {
    Node root;
    Node *fndnode;
    char tkn[1];

    // init tree
    terminals.num_nodes = 0;
    terminals.nodes = malloc(MAX * sizeof(Node*)); // 9! all possible terminal nodes without wins
    root.parent = NULL;
    init_arr(m, n, root.state, 0);
    root.token = 1;
    root.num_childs = m*n;
    root.win = 0;

    // build tree
    goto_childs(&root);
    terminals.nodes = realloc(terminals.nodes, sizeof(Node*)*terminals.num_nodes);

    // go up and weight nodes

    go_up_bf(terminals);

    fndnode = &root;

    print_state(root.state);
    printf("Select your token (X/O, default X):\n");
    scanf("%s", tkn);
    print_help();

    if (tkn[0] == 'O') {
        printf("You are playing for %c\n", tkn[0]);
        while(1) {
            // make a move
            fndnode = &fndnode->childs[idofwinnerchild(fndnode)];
            if (print_state_and_who_wins(fndnode) == 0) {
                break;
            }
            // wait for move
            fndnode = read_input_change_state(&root, fndnode);
            if (print_state_and_who_wins(fndnode) == 0) {
                break;
            }
        }
    } else {
        printf("You are playing for X\n");
        while(1) {
            // wait for move
            fndnode = read_input_change_state(&root, fndnode);
            if (print_state_and_who_wins(fndnode) == 0) {
                break;
            }
            // make a move
            fndnode = &fndnode->childs[idofwinnerchild(fndnode)];
            if (print_state_and_who_wins(fndnode) == 0) {
                break;
            }
        }
    }

    return 0;
}
