#include <iostream> 
#include <database.h>

using namespace std;
  
int main() 
{   
    string db_path = "/app/api/database_reader/data/database.db";

    Database db(db_path.c_str());

    for (image_t img_id = 0; img_id < 20; img_id ++)
    {
        cv::cvKps kps = db.ReadKeyPoints(img_id);
        std::cout << kps.size() << std::endl;

        cv::cvDes des = db.ReadDescriptors(img_id);
        std::cout << des.cols << " " << des.rows << std::endl;
    }
    db.Close();

    return 0; 
} 