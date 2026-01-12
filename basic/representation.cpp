#include "representation.h"

std::vector<std::shared_ptr<Object>> Flatten(const std::shared_ptr<Object>& root) {
    std::shared_ptr<Cell> cur_cell = As<Cell>(root);
    if (cur_cell == nullptr) {
        return {root};
    }
    std::shared_ptr<Object> next;
    std::vector<std::shared_ptr<Object>> answer;
    while (true) {
        answer.push_back(cur_cell->GetFirst());
        auto next = cur_cell->GetSecond();
        auto next_cell = As<Cell>(next);
        if (next_cell == nullptr) {
            answer.push_back(next);
            return answer;
        }
        cur_cell = next_cell;
    }
}
std::shared_ptr<Object> ToAst(const std::vector<std::shared_ptr<Object>>& nodes) {
    if (nodes.empty()) {
        return {};
    }
    if (nodes.size() == 1) {
        return nodes.front();
    }
    std::shared_ptr<Object> root;
    std::shared_ptr<Cell> cur;
    for (size_t i = 0; i + 1 < nodes.size(); ++i) {
        auto new_cell = MakeNode<Cell>();
        if (root == nullptr) {
            root = new_cell;
        }
        if (cur != nullptr) {
            cur->SetSecond(new_cell);
        }
        cur = new_cell;

        cur->SetFirst(nodes[i]);
    }
    cur->SetSecond(nodes.back());
    return root;
}