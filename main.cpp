#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <any>
#include <algorithm>
#include <curl/curl.h>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::istringstream;
using std::vector;
using std::unordered_map;
using std::any;
using std::any_cast;
using std::sort;

// the maximum # of characters istream::ignore() will ignore with delimeter
#define IGNORE_LIMIT 256

class Tag{
    public:
        // the type of tag
        string tag_type;
        // the line of this tag entry
        string line_entry;
        // map of the different fields for a tag. key: field name, value: value of that field
        // utilizes std::any to allow for the map to contain a mix of different value data types
        unordered_map<string, any> field_list;
        
        // returns true if a given field exists in this tag, false if not
        bool has_field(string field){
            if(field_list.count(field)){
                return 1;
            }
            return 0;
        }
};//class Tag
class Tag_EXTM3U: public Tag{
    public:
        Tag_EXTM3U(string tag_type, string line_entry){
            Tag::tag_type = tag_type;
            Tag::line_entry = line_entry;
        }
};//class Tag_EXTM3U
class Tag_EXT_X_INDEPENDENT_SEGMENTS: public Tag{
    public:
        Tag_EXT_X_INDEPENDENT_SEGMENTS(string tag_type, string line_entry){
            Tag::tag_type = tag_type;
            Tag::line_entry = line_entry;
        }
}; //class Tag_EXT_X_INDEPENDENT_SEGMENTS
class Tag_EXT_X_MEDIA: public Tag{
    public:
        Tag_EXT_X_MEDIA(string tag_type, string line_entry, string type, string groupid, string name, string language, string def, string autoselect, string channels, string uri){
            Tag::tag_type = tag_type;
            Tag::line_entry = line_entry;
            field_list["TYPE"] = type;
            field_list["GROUP-ID"] = groupid;
            field_list["NAME"] = name;
            field_list["LANGUAGE"] = language;
            field_list["DEFAULT"] = def;
            field_list["AUTOSELECT"] = autoselect;
            field_list["CHANNELS"] = channels;
            field_list["URI"] = uri;
        }
}; //class Tag_EXT_X_MEDIA
class Tag_EXT_X_STREAM_INF: public Tag{
    public:
        Tag_EXT_X_STREAM_INF(string tag_type, string line_entry, int bandwidth, int avg_bandwidth, string codecs, string resolution, float framerate, string videorange, string audio, string cc){
            Tag::tag_type = tag_type;
            Tag::line_entry = line_entry;
            field_list["BANDWIDTH"] = bandwidth;
            field_list["AVERAGE-BANDWIDTH"] = avg_bandwidth;
            field_list["CODECS"] = codecs;
            field_list["RESOLUTION"] = resolution;
            field_list["FRAME-RATE"] = framerate;
            field_list["VIDEO-RANGE"] = videorange;
            field_list["AUDIO"] = audio;
            field_list["CLOSED-CAPTIONS"] = cc;
        }
}; //class Tag_EXT_X_STREAM_INF
class Tag_EXT_X_I_FRAME_STREAM_INF: public Tag{
    public:
        Tag_EXT_X_I_FRAME_STREAM_INF(string tag_type, string line_entry, int bandwidth, string codecs, string resolution, string videorange, string uri){
            Tag::tag_type = tag_type;
            Tag::line_entry = line_entry;
            field_list["BANDWIDTH"] = bandwidth;
            field_list["CODECS"] = codecs;
            field_list["RESOLUTION"] = resolution;
            field_list["VIDEO-RANGE"] = videorange;
            field_list["URI"] = uri;
        }
}; //class Tag_EXT_X_I_FRAME_STREAM_INF

/*
    download_file() - Uses the systems curl command to download a file with a given url and save with a specified files name
*/
bool download_file(string target_url, string filename){
    // format the curl command
    const string cmd = "curl " + target_url + " --output " + filename;
    // if curl returns any error code, return 1
    if( system(cmd.c_str()) ){
        return 1;
    }
    return 0;
} //download_file()

/*
    parse_document() - Parses the document, creates a corresponding Tag object for each line, and adds it to the list of parsed tags via pass by reference
*/
void parse_document(vector<Tag*> &parsed_tags, string filename){
    ifstream input_file(filename);
    string line;

    while(getline(input_file, line)){
        // ignore lines that are not tag entries
        if(line[0] != '#'){
            continue;
        }
        // use string stream to parse each line
        istringstream line_iss(line);

        // get the tag type of this entry
        string tag_type;
        getline(line_iss, tag_type, ':');
        // create coressponding tag object for this tag type
        if(tag_type == "#EXTM3U"){
            Tag_EXTM3U* new_tag = new Tag_EXTM3U(tag_type, line);
            parsed_tags.push_back(new_tag);
        }
        else if(tag_type == "#EXT-X-INDEPENDENT-SEGMENTS"){
            Tag_EXT_X_INDEPENDENT_SEGMENTS* new_tag = new Tag_EXT_X_INDEPENDENT_SEGMENTS(tag_type, line);
            parsed_tags.push_back(new_tag);
        }
        else if(tag_type == "#EXT-X-MEDIA"){
            string type, groupid, name, language, def, autoselect, channels, uri;

            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, type, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, groupid, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, name, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, language, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, def, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, autoselect, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, channels, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, uri, ',');

            Tag_EXT_X_MEDIA* new_tag = new Tag_EXT_X_MEDIA(tag_type, line, type, groupid, name, language, def, autoselect, channels, uri);
            parsed_tags.push_back(new_tag);
        }
        else if(tag_type == "#EXT-X-STREAM-INF"){
            int bandwidth, avg_bandwidth;
            string buffer, codecs, resolution, videorange, audio, cc;
            float framerate;

            line_iss.ignore(IGNORE_LIMIT, '=');
            getline(line_iss, buffer, ',');
            bandwidth = stoi(buffer);
            line_iss.ignore(IGNORE_LIMIT, '=');
            getline(line_iss, buffer, ',');
            avg_bandwidth = stoi(buffer);
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, codecs, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, resolution, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, buffer, ',');
            framerate = stof(buffer);
            line_iss.ignore(IGNORE_LIMIT, '=');
            getline(line_iss, videorange, ',');
            line_iss.ignore(IGNORE_LIMIT, '=');
            getline(line_iss, audio, ',');
            line_iss.ignore(IGNORE_LIMIT, '=');
            getline(line_iss, cc, ',');

            Tag_EXT_X_STREAM_INF* new_tag = new Tag_EXT_X_STREAM_INF(tag_type, line, bandwidth, avg_bandwidth, codecs, resolution, framerate, videorange, audio, cc);
            parsed_tags.push_back(new_tag);
        }
        else if(tag_type == "#EXT-X-I-FRAME-STREAM-INF"){
            int bandwidth;
            string buffer, codecs, resolution, videorange, uri;

            line_iss.ignore(IGNORE_LIMIT, '=');
            getline(line_iss, buffer, ',');
            bandwidth = stoi(buffer);
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, codecs, ',');
            line_iss.ignore(IGNORE_LIMIT,'=');
            getline(line_iss, resolution, ',');
            line_iss.ignore(IGNORE_LIMIT, '=');
            getline(line_iss, videorange, ',');
            line_iss.ignore(IGNORE_LIMIT, '=');
            getline(line_iss, uri, ',');

            Tag_EXT_X_I_FRAME_STREAM_INF* new_tag = new Tag_EXT_X_I_FRAME_STREAM_INF(tag_type, line, bandwidth, codecs, resolution, videorange, uri);
            parsed_tags.push_back(new_tag);
        }
    }
} //parse_document()

/*
    Comparison functions for sort function
*/
bool compare_bandwidth(Tag* a, Tag* b){
    return (any_cast<int>(a->field_list["BANDWIDTH"]) < any_cast<int>(b->field_list["BANDWIDTH"]) );
}//comparee_bandwidth()

bool compare_codecs(Tag* a, Tag* b){
    return (any_cast<string>(a->field_list["CODECS"]) < any_cast<string>(b->field_list["CODECS"]) );
}//compare_codecs()

bool compare_groupid(Tag* a, Tag* b){
    return (any_cast<string>(a->field_list["GROUP-ID"]) < any_cast<string>(b->field_list["GROUP-ID"]) );
}//compare_codecs()

/*
    sort_by() - Sorts the lists of tags by a desired field and print it out.
*/
void sort_by( unordered_map< string, vector<Tag*> > &tags, string field){
    cout << "*** Sorting by: " << field << " ***" << endl;
    // sort tags with the field we want to sort and print them first
    for(auto& m : tags){
        if(m.second[0]->has_field(field)){
            // determine which comparison function to use based on desired field to sort by
            if(field == "BANDWIDTH"){
                sort(m.second.begin(), m.second.end(), compare_bandwidth);
            }
            else if(field == "CODECS"){
                sort(m.second.begin(), m.second.end(), compare_codecs);
            }
            else if (field == "GROUP-ID"){
                sort(m.second.begin(), m.second.end(), compare_groupid);
            }
            for(auto& i : m.second){
                cout << i->line_entry << endl;
            }
        }
    }
    // print the rest of the tags without the field
    for(auto& m : tags){
        if(!m.second[0]->has_field(field)){
            for(auto& i : m.second){
                cout << i->line_entry << endl;
            }
        }
    }
}//sort_by()


int main(){
    const string target_url = "https://lw.bamgrid.com/2.0/hls/vod/bam/ms02/hls/dplus/bao/master_unenc_hdr10_all.m3u8";
    const string filename = "file.m3u8";

    // flags to determine which field you would like to sort by
    unordered_map<string, bool> sort_choices;
    sort_choices["BANDWIDTH"] = 1;
    sort_choices["CODECS"] = 1;
    sort_choices["GROUP-ID"] = 1;

    // download the file using systems curl command
    if( download_file(target_url, filename) != 0){
        cout << "Error: downloading file failed" << endl;
        return 1;
    }

    // parse the document and create a list of tag objects
    vector<Tag*> parsed_tags;
    parse_document(parsed_tags, filename);

    //catergorize the parsed tags by their tag type using a map: key: tag type, value: list of tag objects of that type
    unordered_map< string, vector<Tag*> > tags;
    for(int i = 0; i < (int)parsed_tags.size(); i++){
        tags[parsed_tags[i]->tag_type].push_back(parsed_tags[i]);
    }

    // sort and print for each of the sort options chosen
    for(auto& i: sort_choices){
        if(i.second == 1){
            sort_by(tags, i.first);
        }
    }

    return 0;
}