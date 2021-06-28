#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
int player;
const int SIZE = 8;
///////////////////////////////////////////////////////////
struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;  //dic_count[3]  紀錄white black empty數量
    int cur_player;
    bool done;
    int winner;
private:
    int get_next_player(int player) const { //下一個player
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const { //點P在不再Board上
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const { //哪種disc在p點
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) { //set disc 在 P點
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const { //特定disc 在不在P點
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const { //center點可不可以放
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))  //如果沒有dir+center的點為對手的旗子 continue
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) { //同直線上的不為空 且不是自己的旗子 就持續運行
                if (is_disc_at(p, cur_player)) //若有自己的旗子->有效的下棋點
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player))) //若非相反顏色的旗子 換一個dir
                continue;
            std::vector<Point> discs({p}); //放要翻轉的東西
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) { //遇到跟自己同色的旗子，反轉discs中Point 的旗子並退出
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p); //遇到和自己相反的旗子 ， 放到discs裡
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard() {
        reset();
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK; //玩家黑旗
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    /////////////////////////////////////////////////////////
    int getscore(int playguy)
    {
        int total = 0;
        for(int i = 0; i < SIZE; ++i)
            for(int j = 0; j < SIZE; ++j)
                if(board[i][j] == playguy)
                    total += 1;
        return total;
    }
    int heuristic(){

    // intialize black and white total
    int b_total = 0;
    int w_total = 0;

    // factor in the amount of moves each player has
    cur_player = 1;
    b_total += get_valid_spots().size()*3;

    cur_player = 2;
    w_total += get_valid_spots().size()*3;

    // factor in the amount of pieces each player has on the board
    b_total += getscore(1)*5;
    w_total += getscore(2)*5;

    // factor in the importance of all 4 corners
    if(board[0][0] == 2){
        w_total += 40;
    }
    if(board[0][0] == 1){
        b_total += 40;
    }
    if(board[7][0] == 2){
        w_total += 40;
    }
    if(board[7][0] == 1){
        b_total += 40;
    }
    if(board[0][7] == 2){
        w_total += 40;
    }
    if(board[0][7] == 1){
        b_total += 40;
    }
    if(board[7][7] == 2){
        w_total += 40;
    }
    if(board[7][7] == 1){
        b_total += 40;
    }
    //
    if(board[2][2]==1) b_total +=4;
    if(board[2][3]==1) b_total +=4;
    if(board[2][4]==1) b_total +=4;
    if(board[2][5]==1) b_total +=4;
    if(board[3][2]==1) b_total +=4;
    if(board[3][5]==1) b_total +=4;
    if(board[4][2]==1) b_total +=4;
    if(board[4][5]==1) b_total +=4;
    if(board[5][2]==1) b_total +=4;
    if(board[5][3]==1) b_total +=4;
    if(board[5][4]==1) b_total +=4;
    if(board[5][5]==1) b_total +=4;

    if(board[2][2]==2) w_total +=4;
    if(board[2][3]==2) w_total +=4;
    if(board[2][4]==2) w_total +=4;
    if(board[2][5]==2) w_total +=4;
    if(board[3][2]==2) w_total +=4;
    if(board[3][5]==2) w_total +=4;
    if(board[4][2]==2) w_total +=4;
    if(board[4][5]==2) w_total +=4;
    if(board[5][2]==2) w_total +=4;
    if(board[5][3]==2) w_total +=4;
    if(board[5][4]==2) w_total +=4;
    if(board[5][5]==2) w_total +=4;

    if(board[0][2]==1) b_total +=8;
    if(board[0][3]==1) b_total +=6;
    if(board[0][4]==1) b_total +=6;
    if(board[0][5]==1) b_total +=8;
    if(board[7][2]==1) b_total +=8;
    if(board[7][3]==1) b_total +=6;
    if(board[7][4]==1) b_total +=6;
    if(board[7][5]==1) b_total +=8;
    if(board[2][7]==1) b_total +=8;
    if(board[3][7]==1) b_total +=6;
    if(board[4][7]==1) b_total +=6;
    if(board[5][7]==1) b_total +=8;
    if(board[2][7]==1) b_total +=8;
    if(board[3][7]==1) b_total +=6;
    if(board[4][7]==1) b_total +=6;
    if(board[5][7]==1) b_total +=8;

    if(board[0][2]==2) w_total +=8;
    if(board[0][3]==2) w_total +=6;
    if(board[0][4]==2) w_total +=6;
    if(board[0][5]==2) w_total +=8;
    if(board[7][2]==2) w_total +=8;
    if(board[7][3]==2) w_total +=6;
    if(board[7][4]==2) w_total +=6;
    if(board[7][5]==2) w_total +=8;
    if(board[2][7]==2) w_total +=8;
    if(board[3][7]==2) w_total +=6;
    if(board[4][7]==2) w_total +=6;
    if(board[5][7]==2) w_total +=8;
    if(board[2][7]==2) w_total +=8;
    if(board[3][7]==2) w_total +=6;
    if(board[4][7]==2) w_total +=6;
    if(board[5][7]==2) w_total +=8;

    if(board[0][1]==1) b_total -=15;
    if(board[1][1]==1) b_total -=15;
    if(board[1][0]==1) b_total -=15;
    if(board[6][0]==1) b_total -=15;
    if(board[6][1]==1) b_total -=15;
    if(board[7][1]==1) b_total -=15;
    if(board[0][6]==1) b_total -=15;
    if(board[1][6]==1) b_total -=15;
    if(board[1][7]==1) b_total -=15;
    if(board[6][6]==1) b_total -=15;
    if(board[6][7]==1) b_total -=15;
    if(board[7][6]==1) b_total -=15;
    // subtract white's total from black, let black be the maximizer
    if(player==1) return (b_total-w_total);
    else return (w_total-b_total);
}
////////////////////////////////////////////////////////
    std::vector<Point> get_valid_spots() const { //掃描整個board每個位置有不有效
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) { //蛤
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }
    std::string encode_player(int state) {
        if (state == BLACK) return "O";
        if (state == WHITE) return "X";
        return "Draw";
    }
    std::string encode_spot(int x, int y) {
        if (is_spot_valid(Point(x, y))) return ".";
        if (board[x][y] == BLACK) return "O";
        if (board[x][y] == WHITE) return "X";
        return " ";
    }
    std::string encode_output(bool fail=false) {
        int i, j;
        std::stringstream ss;
        ss << "Timestep #" << (8*8-4-disc_count[EMPTY]+1) << "\n";
        ss << "O: " << disc_count[BLACK] << "; X: " << disc_count[WHITE] << "\n";
        if (fail) {
            ss << "Winner is " << encode_player(winner) << " (Opponent performed invalid move)\n";
        } else if (next_valid_spots.size() > 0) {
            ss << encode_player(cur_player) << "'s turn\n";
        } else {
            ss << "Winner is " << encode_player(winner) << "\n";
        }
        ss << "+---------------+\n";
        for (i = 0; i < SIZE; i++) {
            ss << "|";
            for (j = 0; j < SIZE-1; j++) {
                ss << encode_spot(i, j) << " ";
            }
            ss << encode_spot(i, j) << "|\n";
        }
        ss << "+---------------+\n";
        ss << next_valid_spots.size() << " valid moves: {";
        if (next_valid_spots.size() > 0) {
            Point p = next_valid_spots[0];
            ss << "(" << p.x << "," << p.y << ")";
        }
        for (size_t i = 1; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << ", (" << p.x << "," << p.y << ")";
        }
        ss << "}\n";
        ss << "=================\n";
        return ss.str();
    }
    std::string encode_state() {
        int i, j;
        std::stringstream ss;
        ss << cur_player << "\n";
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE-1; j++) {
                ss << board[i][j] << " ";
            }
            ss << board[i][j] << "\n";
        }
        ss << next_valid_spots.size() << "\n";
        for (size_t i = 0; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << p.x << " " << p.y << "\n";
        }
        return ss.str();
    }
};
int minimax(int depth, OthelloBoard& board, int alpha, int beta)
{
    if(depth >=6 || board.done==true)
    {
        return board.heuristic();
    }

    if(depth%2==1)
    {
        int max_eval = -9999999;
        std::vector<Point> child = board.get_valid_spots();
        std::vector<Point>::iterator it;
        for(it = child.begin();it!=child.end();it++)
        {
            OthelloBoard next_board = board;
            next_board.put_disc(*it);
            int eval = minimax(depth+1, next_board, alpha, beta);

            max_eval = std::max(max_eval, eval); //////////????????
            alpha = std::max(alpha, eval);

            if(beta <= alpha) break;
        }
        return max_eval;
    }
    else
    { // minimizing layer...
        int min_eval = 9999999; // set min to worst case
        std::vector<Point> child = board.get_valid_spots();
        std::vector<Point>::iterator it;
        for(it = child.begin();it!=child.end();it++)
        {
            OthelloBoard next_board = board;
            next_board.put_disc(*it);
            int eval = minimax(depth+1, next_board, alpha, beta);
            /////////////////???????????????????
            min_eval = std::min(min_eval, eval); // update min if evaluation is <

            // update beta appropriately, and check for eligibility of beta prune
            beta = std::min(beta, eval);
            if(beta <= alpha) break;
        }
        return min_eval;
    }
    return 0;
}
////////////////////////////////////////////////////////////////////

std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;


void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout) {
    srand(time(NULL));

    OthelloBoard Board;
    Board.cur_player = player;
    for(int i = 0;i<8;i++) for(int j = 0;j<8;j++) Board.board[i][j] = board[i][j];
    Point best_point;
    int ans = -999999999;
    std::vector<Point>::iterator it;
    int alpha = -999999999, beta =  999999999;
    for(it = next_valid_spots.begin();it!=next_valid_spots.end();it++)
    {
        OthelloBoard next_board = Board;
        next_board.put_disc(*it);
        int m = 0;
        if(next_board.done == 1)
            m = next_board.heuristic();
        else m = minimax(2, next_board, alpha, beta);

        if(m>ans)
        {
            ans = m;
            best_point = *it;
            alpha = m;
        }
    }
    //if(next_valid_spots)
    fout<<best_point.x<<" "<<best_point.y<<std::endl;
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
