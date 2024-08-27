#include <Controller/View/SearchPage.h>
#include <vector>


EVENTO_UI_START

SearchPage::SearchPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SearchPage::onCreate() {
    auto& self = *this;
    self->on_Searchtext([this](slint::SharedString v){
        std::string txt(v);
        std::vector<std::string> Divisions = {"RunYanBu","DianZiBu","WaiLianBu"}; //搜索框匹配关键词 
    for(int i=0;i<Divisions.size();++i){
        if(!txt.compare(Divisions[i])){
            this->clickFilterDivision(true);
        }
        else{
            std::cout<<"False"<<std::endl;
        }
    }
    });
    self->on_clickFilterDivision([this](bool T){
        if(T==true)
        this->cd_detail_page();
        else
        std::cout<<"failed FilterDivision"<<std::endl;
    });
}
void SearchPage::cd_detail_page(){
    std::cout<<"Hello,World"<<std::endl;
    auto& self=*this;
    self->set_sign_Filter_Division(true);
   //todo
}

void SearchPage::clickFilterDivision(bool T){
    if(true){
    std::cout<<"Hello,Night"<<std::endl;
    }
   /*navigate to*/
   //todo
}
EVENTO_UI_END
