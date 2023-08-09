#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <cassert>
#include <memory>
#include <optional>
#include <bitset>
#define rep(i,n) for(int i = 0; i < (int)(n); i++)

using namespace std;

using LL = long long;
using pii = pair<int,int>;
using vvi = vector<vector<int>>;
constexpr int INF = (int)1e9;
constexpr LL LINF = (LL)1e18;

constexpr int vec_dy[4] = {-1, 1, 0, 0};
constexpr int vec_dx[4] = {0, 0, -1, 1};
const string s_dir = "UDLR";

constexpr int testcase = 0;

enum class file_status{
    local,
    debug,
    score,
    submit,
};
file_status now_file_status = file_status::submit;

void read_input(){
    stringstream ss;
    string num = to_string(testcase);
    int siz = num.size();
    for(int i = 0; i < 4 - siz; i++) num = '0' + num;
    ss << "in/" << num << ".txt";
    FILE *in = freopen(ss.str().c_str(), "r", stdin);
}
void file_output(){
    stringstream ss;
    string num = to_string(testcase);
    int siz = num.size();
    for(int i = 0; i < 4 - siz; i++) num = '0' + num;
    ss << "out/" << num << ".txt";
    FILE *out = freopen(ss.str().c_str(), "w", stdout);
}

random_device rnd;
mt19937 engine(rnd());
uniform_int_distribution<> randInt(0, 1);

// ----------zobrist hash start----------
// 必要に応じてセルの種類を追加
enum Cell {
    EMPTY,
    CELL_KINDS,
};
using vvc = vector<vector<Cell>>;
using uint = unsigned int;

vector<uint> hash_list;
uniform_int_distribution<> hash_gen(1, (int)1e9);
void hash_init(const int grid_size){
	for(int i = 0; i < grid_size * CELL_KINDS; i++) {
		uint val1 = hash_gen(engine);
		hash_list.emplace_back(val1);
	}
}
uint cell_hash(int position, int kind) {
    int number = position * CELL_KINDS + kind;
    return hash_list[number];
}
uint grid_hash(const vvc& grid) {
    assert(!grid.empty());
    const int row_size = grid.size();
    const int column_size = grid[0].size();
    uint res = 0;

	for(int i = 0; i < row_size; i++) {
		for(int j = 0; j < column_size; j++) {
            int pos = i * column_size + j;
            int kind = grid[i][j];
			res ^= cell_hash(pos, kind);
		}
	}
	return res;
}
uint update_grid_hash(int position, int kind, int new_kind, uint now_hash){
    uint res = now_hash;
    res ^= cell_hash(position, kind);
    res ^= cell_hash(position, new_kind);
	return res;
}
// ----------zobrist hash end----------

void input() {

}

struct Operation {
    int move_dir;
    Operation(int move_dir) : move_dir(move_dir) {}
};
vector<Operation> valid_operations;
void SetOperations() {
    for(int dir = 0; dir < 4; dir++) {
        valid_operations.emplace_back(dir);
    }
}

void Initialize() {
    input();
    SetOperations();
    constexpr int grid_size = 2500;
    hash_init(grid_size);
}

void output(vector<Operation>& result) {
    const string str_dir = "UDLR";
    for(auto& op : result) {
        std::cout << str_dir[op.move_dir];
    }
    std::cout << std::endl;
}

struct Restore {
    // ToDo
};
Restore::Restore() {
    // ToDo
}

struct Node {
    Operation op;
    Node* parent;
    vector<Node*> children;
    bool expired;

    Node(Operation op, Node* parent);
};
Node::Node(Operation op, Node* parent) : op(op), parent(parent) {
    expired = false;
}

struct TemporaryNode {
    int raw_score, eval_score;
    uint hash;
    Operation op;
    Node* parent;

    TemporaryNode(int raw_score, int eval_score, uint& hash, Operation op);
};
TemporaryNode::TemporaryNode(int raw_score, int eval_score, uint& hash, Operation op) :
raw_score(raw_score), eval_score(eval_score), hash(hash), op(op) {
    parent = nullptr;
}

struct State {
    // ToDo

    State();
    int score() const;
    uint hash() const;
    TemporaryNode try_move(const Operation& op) const;
    Restore apply_move(const Operation& op);

    void roll_back(const Restore& res, const Operation& op);
};
State::State() {
    // ToDo
}
int State::score() const {
    // ToDo
}
uint State::hash() const {
    // ToDo
}
TemporaryNode State::try_move(const Operation& op) const {
    // ToDo
}
Restore State::apply_move(const Operation& op) {
    // ToDo
}
void State::roll_back(const Restore& res, const Operation& op) {
    // ToDo
}

struct Tree {
    State state;
    Node* root_node;

    Tree(State& state);
    void dfs(Node* node_ptr, vector<TemporaryNode>& temp_nodes, bool single);
};
Tree::Tree(State& state) : state(state) {
    root_node = new Node(Operation{-1}, nullptr);
}
void Tree::dfs(Node* node_ptr, vector<TemporaryNode>& temp_nodes, bool single) {
    // 子ノードが存在しないなら葉なので、候補を追加して終了
    if(node_ptr->children.empty()) {
        for(const auto& op : valid_operations) {
            temp_nodes.emplace_back(state.try_move(op));
            temp_nodes.back().parent = node_ptr;
        }
        node_ptr->expired = true;
        return;
    }

    // 使われない子ノードを削除
    node_ptr->children.erase(remove_if(node_ptr->children.begin(), node_ptr->children.end(),
    [](Node* child_ptr) { return child_ptr->expired; }), node_ptr->children.end());
    bool next_single = single && ((int)node_ptr->children.size() == 1);

    auto node_backup = node_ptr;
    // 残った子ノードを走査
    for(auto& child_ptr : node_ptr->children) {
        Restore res = state.apply_move(child_ptr->op);
        dfs(child_ptr, temp_nodes, next_single);
        if(!next_single) state.roll_back(res, child_ptr->op);
    }
    if(!next_single) root_node = node_backup;

    // ノードを不要としておき、必要なら後で復活させる
    node_ptr->expired = true;
}

vector<Operation> BeamSearch(const int max_depth, const int beam_width) {
    State init_state;
    Tree tree(init_state);

    vector<TemporaryNode> final_nodes;

    unordered_set<uint> fields;
    vector<TemporaryNode> temp_nodes;

    for(int turn = 1; turn <= max_depth; turn++) {
        fields.clear();
        temp_nodes.clear();

        tree.dfs(tree.root_node, temp_nodes, true);
        // 最後のターンなら一時ノードの情報を保存して終了
        if(turn == max_depth) {
            final_nodes = temp_nodes;
            break;
        }

        int node_size = temp_nodes.size();
        // 候補がビーム幅より多いなら上位beam_width個を選ぶ
        if(node_size > beam_width) {
            nth_element(temp_nodes.begin(), temp_nodes.begin() + beam_width, temp_nodes.end(),
            [](TemporaryNode& n1, TemporaryNode& n2) {
                return n1.eval_score > n2.eval_score;
            });
        }
        // 仮ノードの情報から実際にノードを更新する
        for(int i = 0; i < min(beam_width, node_size); i++) {
            if(fields.count(temp_nodes[i].hash)) continue;
            fields.insert(temp_nodes[i].hash);
            temp_nodes[i].parent->children.emplace_back(
                new Node(temp_nodes[i].op, temp_nodes[i].parent)
            );

            // 採用されたノードから親をたどりノードを復活させる
            Node* node_ptr = temp_nodes[i].parent;
            while(node_ptr && node_ptr->expired) {
                node_ptr->expired = false;
                node_ptr = node_ptr->parent;
            }
        }
    }

    // 最良の状態を選択
    int arg_best = -1;
    int best_score = 0;
    for(int i = 0; i < (int)final_nodes.size(); i++) {
        if(final_nodes[i].raw_score > best_score) {
            arg_best = i;
            best_score = final_nodes[i].raw_score;
        }
    }
    assert(arg_best != -1);
    Operation op = final_nodes[arg_best].op;
    auto ptr = final_nodes[arg_best].parent;

    vector<Operation> result{op};
    std::cerr << "raw_score = " << best_score << endl;

    // 操作の復元
    while(ptr->parent) {
        result.emplace_back(ptr->op);
        ptr = ptr->parent;
    }
    reverse(result.begin(), result.end());
    return result;
}

int main(){
    if(now_file_status != file_status::submit){
        read_input();
        file_output();
    }
    Initialize();

    const int max_depth = 2500, beam_width = 1500;
    auto result = BeamSearch(max_depth, beam_width);
    output(result);

	return 0;
}
