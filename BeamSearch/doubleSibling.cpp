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
    // ToDo
};
vector<Operation> valid_operations;
void SetOperations() {

}

void Initialize() {
    input();
    SetOperations();
    constexpr int grid_size = 2500;
    hash_init(grid_size);
}

void output(vector<Operation>& result) {

}

struct Restore {
    // ToDo
    Restore();
};
Restore::Restore() {
    // ToDo
}

struct Node {
    Operation op;
    Node* parent;
    Node* child;
    Node* prev_sibling;
    Node* next_sibling;

    Node();
    void initialize(Operation op, Node* parent);
};
Node::Node() {
    //op = nullopt;
    parent = nullptr;
    child = nullptr;
    prev_sibling = nullptr;
    next_sibling = nullptr;
}
void Node::initialize(Operation op, Node* parent) {
    this->op = op;
    this->parent = parent;
    child = nullptr;
    prev_sibling = nullptr;
    next_sibling = nullptr;
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
    TemporaryNode try_move(const Operation& op) const;
    Restore apply_move(const Operation& op);

    void roll_back(const Restore& res, const Operation& op);
};
State::State() {
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
    root_node = nullptr;
}
void Tree::dfs(Node* node_ptr, vector<TemporaryNode>& temp_nodes, bool single) {
    if(!node_ptr->child) {
        for(const auto& op : valid_operations) {
            temp_nodes.emplace_back(state.try_move(op));
            temp_nodes.back().parent = node_ptr;
        }
        return;
    }

    // 一本道なら状態を戻さない
    bool next_single = single && (!node_ptr->child->next_sibling);

    auto node_backup = node_ptr;
    // 残っている子ノードを走査
    Node* child_ptr = node_ptr->child;
    while(child_ptr) {
        Restore res = state.apply_move(child_ptr->op);
        dfs(child_ptr, temp_nodes, next_single);
        if(!next_single) state.roll_back(res, child_ptr->op);
        child_ptr = child_ptr->next_sibling;
    }
    if(!next_single) root_node = node_backup;
}

vector<Operation> BeamSearch(const int max_depth, const int beam_width) {
    State init_state;
    Tree tree(init_state);

    vector<TemporaryNode> final_nodes, temp_nodes;
    vector<Node*> current_nodes, next_nodes;

    constexpr int max_nodes = 15000; // 要調整
    vector<Node> valid_nodes(max_nodes);
    tree.root_node = &valid_nodes[0];
    vector<Node*> vacant;
    for(int i = max_nodes - 1; i > 0; i--) {
        vacant.emplace_back(&valid_nodes[i]);
    }

    unordered_set<uint> fields;

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

            Node* parent = temp_nodes[i].parent;
            /*
            // 必要ならばノード数が足りているか確認する
            if(vacant.empty()) {
                std::cerr << "max_nodes を大きくしてください" << std::endl;
            }
            assert(!vacant.empty());
            */
            // 既に子がいるなら新たに代表の子として挿入する
            if(parent->child) {
                parent->child->prev_sibling = vacant.back();
                vacant.back()->initialize(temp_nodes[i].op, parent);

                parent->child->prev_sibling->next_sibling = parent->child;
                parent->child = parent->child->prev_sibling;
            }
            // 子がいないなら代表の子とする
            else {
                parent->child = vacant.back();
                vacant.back()->initialize(temp_nodes[i].op, parent);
            }
            next_nodes.emplace_back(vacant.back());
            vacant.pop_back();
        }
        // 子がいないノードを再帰的に削除する
        for(auto ptr : current_nodes) {
            while(!ptr->child) {
                // 前も後ろも兄弟がいる場合
                if(ptr->prev_sibling && ptr->next_sibling) {
                    ptr->prev_sibling->next_sibling = ptr->next_sibling;
                    ptr->next_sibling->prev_sibling = ptr->prev_sibling;
                    vacant.emplace_back(ptr);
                    break;
                }
                // 前の兄弟だけいる場合
                else if(ptr->prev_sibling && !ptr->next_sibling) {
                    ptr->prev_sibling->next_sibling = nullptr;
                    vacant.emplace_back(ptr);
                    break;
                }
                // 後ろの兄弟だけいる場合
                else if(!ptr->prev_sibling && ptr->next_sibling) {
                    ptr->next_sibling->prev_sibling = nullptr;
                    ptr->parent->child = ptr->next_sibling;
                    vacant.emplace_back(ptr);
                    break;
                }
                // 両方いない場合はさらに親も削除する
                else {
                    if(!ptr->parent) {
                        vacant.emplace_back(ptr);
                        break;
                    }
                    else {
                        vacant.emplace_back(ptr);
                        ptr = ptr->parent;
                        ptr->child = nullptr;
                    }
                }
            }
        }

        swap(current_nodes, next_nodes);
        next_nodes.clear();
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

    const int max_depth = 2500, beam_width = 3000;
    auto result = BeamSearch(max_depth, beam_width);
    output(result);

	return 0;
}
