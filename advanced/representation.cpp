#include "representation.h"

std::vector<ObjectPtr> Flatten(const ObjectPtr& root) {
    std::shared_ptr<Cell> cur_cell = As<Cell>(root);
    if (cur_cell == nullptr) {
        return {root};
    }

    ObjectPtr next;
    std::vector<ObjectPtr> answer;

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

ObjectPtr ToAst(const std::vector<ObjectPtr>& nodes) {
    if (nodes.size() == 1) {
        return nodes.front();
    }

    ObjectPtr root;
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