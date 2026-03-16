#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>
#include <bmp_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>

using namespace std;

enum class Format{
    PPM,
    JPEG,
    BMP,
    UNKNOWN
};

class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};

class PPM : public ImageFormatInterface{
public:
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override{
        return img_lib::SavePPM(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const override{
        img_lib::Image image =  img_lib::LoadPPM(file);
        return image;
    }
};

class JPEG : public ImageFormatInterface{
public:
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override{
        return img_lib::SaveJPEG(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const override{
        img_lib::Image image = img_lib::LoadJPEG(file);
        return image;
    }
};

class BMP : public ImageFormatInterface{
public:
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override{
        return img_lib::SaveBMP(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const override{
        img_lib::Image image = img_lib::LoadBMP(file);
        return image;
    }
};

Format GetFormatByExtension(const img_lib::Path& input_file) {
    using namespace std::literals;
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    if (ext == ".bmp"sv) {
        return Format::BMP;
    }

    return Format::UNKNOWN;
}

ImageFormatInterface* GetFormatInterface(const img_lib::Path& input_file){
    Format file_format = GetFormatByExtension(input_file);
    if(file_format == Format::JPEG){
        ImageFormatInterface* jpeg_format = new JPEG();
        return jpeg_format;
    }else if(file_format == Format::PPM){
        ImageFormatInterface* ppm_format = new PPM();
        return ppm_format;
    }else if(file_format == Format::BMP){
        ImageFormatInterface* bmp_format = new BMP();
        return bmp_format;
    }else{
        return nullptr;
    }
}

int main(int argc, const char** argv) {
    using namespace std::literals;
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    ImageFormatInterface* input_interface = GetFormatInterface(in_path);
    if(input_interface == nullptr){
        cerr << "Unknown format of the input file"sv << endl;
        return 2;
    }
    ImageFormatInterface* out_interface = GetFormatInterface(out_path);
    if(out_interface == nullptr){
        cerr << "Unknown format of the output file"sv << endl;
        return 3;
    }

    img_lib::Image image = input_interface->LoadImage(in_path);
    if (!image) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!out_interface->SaveImage(out_path, image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;
}