#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <random>
#include <cassert>
#define rep(i,n) for(int i = 0; i < (int)(n); i++)

using namespace std;
using LL = long long;
using pii = pair<int,int>;
using vvi = vector<vector<int>>;
constexpr int INF = (int)1e9;
constexpr LL LINF = (LL)1e18;

constexpr int vec_dx[4] = {0, 0, -1, 1};
constexpr int vec_dy[4] = {-1, 1, 0, 0};
const string dir_str = "UDLR";

constexpr int testcase = 0;

enum class file_status{
    local,
    debug,
    score,
    submit,
};
file_status now_file_status = file_status::local;

void read_input(){
    std::stringstream ss;
    std::string num = std::to_string(testcase);
    int siz = num.size();
    for(int i = 0; i < 4 - siz; i++) num = '0' + num;
    ss << "in/" << num << ".txt";
    freopen(ss.str().c_str(), "r", stdin);
}
void file_output(){
    std::stringstream ss;
    std::string num = std::to_string(testcase);
    int siz = num.size();
    for(int i = 0; i < 4 - siz; i++) num = '0' + num;
    ss << "out/" << num << ".txt";
    freopen(ss.str().c_str(), "w", stdout);
}

/* ビームサーチライブラリ
ToDo List:
・enum Cellにセルの種類を追加 (ハッシュ値が必要なら)
・input関数で入力の受け取り
・操作をOperation構造体で定義、可能な全ての操作をSetOperations関数で列挙
・問題に応じてState構造体を定義
・ビーム幅やグリッドの大きさなどの数値設定
*/

random_device rnd;
mt19937 engine(rnd());
uniform_real_distribution<> randR(0.0, 1.0);

// ----------zobrist hash start----------
// 必要に応じてセルの種類を追加
enum Cell {
    EMPTY,
    CELL_KINDS,
};
using vvc = vector<vector<Cell>>;
using ull = unsigned long long;

vector<ull> hash_list;
uniform_int_distribution<> hash_gen(1, (int)1e9);
void hash_init(const int grid_size){
	for(int i = 0; i < grid_size * CELL_KINDS; i++) {
		ull val1 = hash_gen(engine);
		ull val2 = hash_gen(engine);
		hash_list.emplace_back(val1 * val2);
	}
}
ull cell_hash(int position, int kind) {
    int number = position * CELL_KINDS + kind;
    return hash_list[number];
}
ull grid_hash(const vvc& grid) {
    assert(!grid.empty());
    const int row_size = grid.size();
    const int column_size = grid[0].size();
    ull res = 0;

	for(int i = 0; i < row_size; i++) {
		for(int j = 0; j < column_size; j++) {
            int pos = i * column_size + j;
            int kind = grid[i][j];
			res ^= cell_hash(pos, kind);
		}
	}
	return res;
}
ull update_grid_hash(int position, int kind, int new_kind, ull now_hash){
    ull res = now_hash;
    res ^= cell_hash(position, kind);
    res ^= cell_hash(position, new_kind);
	return res;
}
// ----------zobrist hash end----------

void input() {

}

struct Operation {

};
vector<Operation> valid_operations;
void SetOperations() {

}

void Initialize() {
    input();
    SetOperations();
    constexpr int grid_size = 2500; // 問題に応じて変更する
    hash_init(grid_size);
}

struct State {
    // コピーすべき情報をここに書く

    State();
    int score() const;
    ull hash() const;
    pair<int,ull> try_move(const Operation& op) const;
    void apply_move(const Operation& op);
};
State::State() {

}
int State::score() const {

}
ull State::hash() const {

}
// 一手進めた場合のスコアとハッシュ値を返す、更新はしない
pair<int,ull> State::try_move(const Operation& op) const {

}
// 更新する
void State::apply_move(const Operation& op) {

}

// 行動を復元する永続stack
struct History {
    Operation op;
    shared_ptr<History> parent;
    History(const Operation& op, shared_ptr<History>& parent) : op(op), parent(parent) {}
};
struct Stack {
    shared_ptr<History> head;

    Operation top();
    Stack push(const Operation& op);
    Stack pop();
};
Operation Stack::top() {
    return head->op;
}
Stack Stack::push(const Operation& op) {
    return Stack({make_shared<History>(op, head)});
}
Stack Stack::pop() {
    return Stack({head->parent});
}

struct Node {
    State state;
    Stack move_history;

    Node(State& state);
    int get_score() const;
    ull get_hash() const;
    pair<int,ull> calculate(const Operation& op) const;
    void advance(const Operation& op);
};
Node::Node(State& state) : state(state) {}
int Node::get_score() const {
    return state.score();
}
ull Node::get_hash() const {
    return state.hash();
}
pair<int,ull> Node::calculate(const Operation& op) const {
    return state.try_move(op);
}
void Node::advance(const Operation& op) {
    state.apply_move(op);
}

// スコアだけ計算して上位を選ぶために用いる仮ノード
struct TemporaryNode {
    int score;
    ull hash;
    int node_index;
    Operation op;
    double rand; // タイブレーク用

    TemporaryNode(int score, ull hash, int node_index, Operation& op);
};
TemporaryNode::TemporaryNode(int score, ull hash, int node_index, Operation& op) :
score(score), hash(hash), node_index(node_index), op(op) {
    rand = randR(engine);
}

Node BeamSearch(State& init_state, const int max_depth, const int beam_width) {
    vector<Node> nodes, next_nodes;
    nodes.emplace_back(init_state);
    nodes.back().move_history = Stack{nullptr};

    vector<TemporaryNode> temp_nodes; // スコア比較用の仮ノードを保管
    unordered_set<ull> fields; // 重複除去用

    for(int turn = 1; turn <= max_depth; turn++) {
        temp_nodes.clear();
        fields.clear();
        
        for(int i = 0; i < (int)nodes.size(); i++) {
            // 可能な全ての遷移を試す
            for(auto& op : valid_operations) {
                auto [next_score, next_hash] = nodes[i].calculate(op);
                temp_nodes.emplace_back(next_score, next_hash, i, op);

                // 必要なら重複除去
                if(fields.count(temp_nodes.back().hash)) {
                    temp_nodes.pop_back();
                }
                else {
                    fields.insert(temp_nodes.back().hash);
                }
            }
        }

        int node_size = temp_nodes.size();
        // 候補がビーム幅より多いなら上位beam_width個を選ぶ
        if(node_size > beam_width) {
            nth_element(temp_nodes.begin(), temp_nodes.begin() + beam_width, temp_nodes.end(),
            [](TemporaryNode& n1, TemporaryNode& n2) {
                if(n1.score == n2.score) {
                    return n1.rand > n2.rand;
                }
                return n1.score > n2.score;
            });
        }

        // 仮ノードの情報から実際にノードを更新する
        for(int i = 0; i < min(beam_width, node_size); i++) {
            int index = temp_nodes[i].node_index;
            next_nodes.emplace_back(nodes[index]);
            next_nodes.back().advance(temp_nodes[i].op);
            // 必要ならスコアとハッシュ値を確認
            // assert(next_nodes.back().score == temp_nodes[i].score);
            // assert(next_nodes.back().hash == temp_nodes[i].hash);

            // 親ノードのスタックに操作を追加して新しいノードのスタックを作成する
            next_nodes.back().move_history = nodes[index].move_history.push(temp_nodes[i].op);
        }

        swap(nodes, next_nodes);
        next_nodes.clear();
    }

    int arg_best = -1, best_score = 0;
    for(int i = 0; i < (int)nodes.size(); i++) {
        if(nodes[i].get_score() > best_score) {
            arg_best = i;
            best_score = nodes[i].get_score();
        }
    }
    return nodes[arg_best];
}

int main(){
    if(now_file_status != file_status::submit){
        read_input();
        file_output();
    }
    Initialize();
    
    State init_state;
    constexpr int max_depth = 2500, beam_width = 100;
    Node result = BeamSearch(init_state, max_depth, beam_width);

    vector<Operation> moves;
    Stack move_history = result.move_history;
    while(move_history.head) {
        Operation op = move_history.top();
        moves.emplace_back(op);
        move_history = move_history.pop();
    }
    reverse(moves.begin(), moves.end());

	return 0;
}
