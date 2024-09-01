#include <Controller/View/DiscoveryPage.h>

EVENTO_UI_START

DiscoveryPage::DiscoveryPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}
    

// inline int getBlockColNum(int self_width, int block_max_w) {
//     int res = self_width / block_max_w;
//     return res;
// }
// 
// inline int getBlockRowNum(int self_height, int block_max_h) {
//     int res = self_height / block_max_h;
//     return res;
// }
// 
// inline int calcBlockX(int idx, int block_width, int block_col_num){
//     return block_width * (idx % block_col_num);
// }
// 
// inline int calcBlockY(int idx, int block_height, int block_col_num) {
//     return block_height * (idx / block_col_num);
// }

void DiscoveryPage::onCreate() {
    auto& self = *this;
    self->on_div([](int a, int b){ return a / b; });
    self->on_calc_block_col_num([](int self_width, int block_max_w){
        int res = self_width / block_max_w;
        return res;
    });
    self->on_calc_block_row_num([](int self_height, int block_max_h){
        int res = self_height / block_max_h;
        return res;
    });
}

EVENTO_UI_END
