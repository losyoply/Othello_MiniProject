#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include<string>
#include <stdexcept>
#include <regex>
struct Point
{
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
};
int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
int easy_board[SIZE][SIZE];

///////////////////////////////////////////////////////////
bool isGameOver(int board[8][8]);
int heuristic(int board[8][8]);
void makeMove(int (&board)[8][8], Point p, int playguy);
std::vector<Point> getWhiteLegalMoves(int board[8][8]);
std::vector<Point> getBlackLegalMoves(int board[8][8]);
struct Node
{
    Node ** child;
    int child_count;
    std::vector<Point> move_list;
    int state[SIZE][SIZE];
    int val;
};
Node * CreateTree(int board[SIZE][SIZE], int depth, int playguy)
{
    Node * node = new Node();
    node->move_list = (playguy==2)?getWhiteLegalMoves(board):getBlackLegalMoves(board);

    node->child_count = node->move_list.size();

    std::memcpy(node->state, board, SIZE * SIZE * sizeof(int));

    int other_player = (playguy == 1) ? 2 : 1;

    if (depth > 0 && node->child_count > 0)
    {
        node->child = new Node * [node->child_count];

        for (int i = 0; i < node->child_count; ++i)
        {
            int tmp_board[8][8];
            std::memcpy(tmp_board, board, SIZE * SIZE * sizeof(char));

            makeMove(tmp_board, node->move_list[i], playguy);

            node->child[i] = CreateTree(tmp_board, depth - 1, other_player);
        }
    }
    else
    {
        node->child = NULL;
    }

    return node;
}
// crucial minimax method for making smart AI choices (other methods may be added in the future)
int minimax(Node *position, int depth, int alpha, int beta, bool maximizing_player)
{
    if(depth == 0 || isGameOver(position->state))
    {
        return heuristic(position->state);
    }

    if(maximizing_player)
    {
        int max_eval = -9999999;

        for(int i = 0; i < position->child_count; ++i)
        {
            int eval = minimax(position->child[i], depth - 1, alpha, beta, false);
            max_eval = std::max(max_eval, eval);

            alpha = std::max(alpha, eval);

            if(beta <= alpha) //prune
            {
                break;
            }
        }
        position->val = max_eval; // store the max_eval in this node
        return max_eval;
    }
    else
    { // minimizing layer...
        int min_eval = 9999999; // set min to worst case
        for(int i = 0; i < position->child_count; ++i)
        {
            int eval = minimax(position->child[i], depth -1, alpha, beta, true);
            min_eval = std::min(min_eval, eval); // update min if evaluation is <

            // update beta appropriately, and check for eligibility of beta prune
            beta = std::min(beta, eval);
            if(beta <= alpha)
                break;
        }
        position->val = min_eval; // store min_eval in this node
        return min_eval;
    }
}
int getScore(int board[SIZE][SIZE], int playguy){
    int total = 0;
    for(int i = 0; i < SIZE; ++i)
        for(int j = 0; j < SIZE; ++j)
            if(board[i][j] == playguy)
                total += 1;

    return total;
}
// heursitic used to give value to varying states of the game
int heuristic(int board[8][8]){

    // intialize black and white total
    int b_total = 0;
    int w_total = 0;

    // factor in the amount of moves each player has
    b_total += getBlackLegalMoves(board).size();
    w_total += getWhiteLegalMoves(board).size();

    // factor in the amount of pieces each player has on the board
    b_total += getScore(board, 1);
    w_total += getScore(board, 2);

    // factor in the importance of all 4 corners
    if(board[0][0] == 2){
        w_total += 10;
    }
    if(board[0][0] == 1){
        b_total += 10;
    }
    if(board[7][0] == 2){
        w_total += 10;
    }
    if(board[7][0] == 1){
        b_total += 10;
    }
    if(board[0][7] == 2){
        w_total += 10;
    }
    if(board[0][7] == 1){
        b_total += 10;
    }
    if(board[7][7] == 2){
        w_total += 10;
    }
    if(board[7][7] == 1){
        b_total += 10;
    }
    // subtract white's total from black, let black be the maximizer
    return (b_total-w_total);
}
bool isGameOver(int board[8][8]){
    return getBlackLegalMoves(board).empty() && getWhiteLegalMoves(board).empty();
}
void flip(int (&board)[8][8], Point p, int playguy){
    // declare a list of positions of discs that will be flipped
    // e.g. {{0,1}, {0,2}} means disc at location board[0][1] & board[0][2] will be flipped
    std::vector<Point> discs_to_flip;

    int otherPlayer = (playguy == 1) ? 2 : 1;

    // use deltas to find all 8 surrounding positions
    std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};

    // for every delta representing a neighboring position...
    for(auto dir : directions){
        //std::cout << "deltas: [" << deltas[0] << ", " << deltas[1] << "]" << '\n';

        // save what row/col currently on
        int curr_row = p.x + dir.x;
        int curr_col = p.y + dir.y;

        // ignore if this goes off of the board
        if(curr_row > 7 || curr_row < 0 || curr_col > 7 || curr_col < 0)
            continue;


        // save character in this position
        int char_in_pos = board[curr_row][curr_col];

        // use this variable to save whether or not a line of pieces should be flipped
        bool flip_this_direction = false;

        // if the character in this delta position is the opponent's piece...
        if(char_in_pos == otherPlayer){
            //std::cout << "Found other player at location: [" << curr_row << ", " << curr_col << "], " << char_in_pos << '\n';

            // continue in this delta position until the next character is no longer the opponent's or you go off the board
            while(char_in_pos == otherPlayer)
            {
                curr_row += dir.x;
                curr_col += dir.y;

                // check to see if new position is off board
                if(curr_row > 7 || curr_row < 0 || curr_col > 7 || curr_col < 0)
                    break;

                // save the character
                char_in_pos = board[curr_row][curr_col];
            }

            // if the player's piece is found after traversing over the opponent's piece(s), we know we will be flipping
            if(char_in_pos == playguy)
                flip_this_direction = true;

            // if we found out we should be flipping...
            if(flip_this_direction){
                // save current position
                curr_row = p.x + dir.x;
                curr_col = p.y + dir.y;
                char_in_pos = board[curr_row][curr_col];

                // traverse over the opponent's pieces, while saving the positions to the big list to be flipped later
                while(char_in_pos == otherPlayer)
                {
                    //std::cout << "flipping [" << curr_row << ", " << curr_col << "]\n";
                    Point P = Point(curr_row,curr_col);
                    discs_to_flip.push_back(P);
                    curr_row += dir.x;
                    curr_col += dir.y;

                    // save next character
                    char_in_pos = board[curr_row][curr_col];
                }

            }
        }
    }

    // after we've collecting the row/col of all discs to flipped, flip them to the current player's color/character
    for(auto points : discs_to_flip)
        board[points.x][points.y] = playguy;
}
void makeMove(int (&board)[8][8], Point p, int playguy){
    //std::cout << "Updating row: " << row << " col: " << col << '\n';
    // set provided row/col position to the player's character piece
    board[p.x][p.y] = playguy;

    // flip discs from resulting move
    flip(board, p, playguy);
}
bool isFlippable(int board[8][8], Point p, int playguy) {
    int otherPlayer = (playguy == 1) ? 2 : 1;

    // Check all 8 surround positions
    std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};

    // for every delta of the surrounding positions
    for(auto dir : directions)
    {
        // skip if the position is off of game board
        if(p.x+dir.x > 7 || p.x+dir.x < 0 || p.y+dir.y > 7 || p.y+dir.y < 0)
        {
            continue;
        }

        //std::cout << "deltas: [" << deltas[0] << ", " << deltas[1] << "]" << '\n';
        int char_in_pos = board[p.x+dir.x][p.y+dir.y]; // grab the character in that spot

        // if the character in this delta spot is the opponent's piece...
        if(char_in_pos == otherPlayer)
        {
            // save spot's row and col #
            int curr_row = p.x + dir.x;
            int curr_col = p.y + dir.y;

            //std::cout << "Found other player at location: [" << curr_row << ", " << curr_col << "], " << char_in_pos << '\n';

            //continue along this delta trajectory until you stop seeing the opponent's pieces
            while(char_in_pos == otherPlayer){
                curr_row += dir.x;
                curr_col += dir.y;

                // check to see if new position is off board
                if(curr_row > 7 || curr_row < 0 || curr_col > 7 || curr_row < 0)
                    break;

                // save the next character
                char_in_pos = board[curr_row][curr_col];
            }

            // if the player's piece is seen after one (+more) of the opponent's pieces, the original move is a flippable one
            if(char_in_pos == playguy)
                return true;
        }
    }

    // if no flippable spot is found after checking all surrounding positions, the original move is not a flippable one
    return false;
}
std::vector<Point> calculateLegalMoves(int board[8][8], int playguy) {

    // declare main move list
    std::vector<Point> move_list;

    for(int i = 0; i < SIZE; ++i)
    {
        for(int j = 0; j < SIZE; ++j)
        {
            // first make sure the spot is empty
            if(board[i][j] == 0)
            {
                // check to see if placing a piece there will flip one (+more) of the opponent's pieces
                Point p = Point(i, j);
                if(isFlippable(board, p, playguy))
                {
                    // if so, create a 2-element vector representative of the move and push it to the big move list
                    move_list.push_back(p);
                }

            }
        }
    }
    return move_list;
}

std::vector<Point> getBlackLegalMoves(int board[8][8]) {
    return calculateLegalMoves(board, 1);
}

// return a list of all the moves available to white
std::vector<Point> getWhiteLegalMoves(int board[8][8]) {
    return calculateLegalMoves(board, 2);
}

Point finddiffpoint(int state[8][8])
{

    for(int i = 0;i<8;i++)
    {
        for(int j = 0;j<8;j++)
        {
            if(easy_board[i][j]!=state[i][j])
            {
                return Point(i, j);
            }
        }
    }
}
///////
/////////////////////////////////////////
void read_board(std::ifstream& fin)
{
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
            easy_board[i][j] = board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin)
{
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout) {
    //int n_valid_spots = next_valid_spots.size();
    int flag = 0;
    auto gametree = CreateTree(easy_board, 5, player);
    bool maximizer = (player == 1) ? true : false;
    int optimial_val = minimax(gametree, 5, -99999999, 99999999, maximizer);
    for(int i = 0; i < gametree->child_count; ++i)
    {
        //std::cout << gametree->children[i]->val << '\n';
        if(gametree->child[i]->val == optimial_val)
        {

                Point p = finddiffpoint(gametree->child[i]->state);
                fout << p.x << " " << p.y << std::endl;
                flag = 1;

        }

    }
    if(flag==0)
    {
        Point p = gametree->move_list[1];
        fout << p.x << " " << p.y << std::endl;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //int n_valid_spots = next_valid_spots.size();
    //srand(time(NULL));
    // Choose random spot. (Not random uniform here)
    //int index = (rand() % n_valid_spots);
    //Point p = next_valid_spots[index];
    // Remember to flush the output to ensure the last action is written to file.
    //fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}

