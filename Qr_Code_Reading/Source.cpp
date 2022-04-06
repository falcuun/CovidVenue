/*
OpenCV 4.5.5 https://sourceforge.net/projects/opencvlibrary/files/4.5.5/opencv-4.5.5-vc14_vc15.exe/download
SHA256 https://github.com/System-Glitch/SHA256
*/

#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "SHA256.h"


/*
struct UserData
{
	std::string name;
	std::string lastName;
	std::string fullAddress;
	std::string dateOfBirth;
	std::string vaccinationStatus;
	std::string testStatus;

	UserData(std::string name, std::string lastName, std::string fullAddress, std::string dateOfBirth, std::string vaccinationStatus, std::string testStatus)
	{
		this->name = name;
		this->lastName = lastName;
		this->fullAddress = fullAddress;
		this->dateOfBirth = dateOfBirth;
		this->vaccinationStatus = vaccinationStatus;
		this->testStatus = testStatus;
	}

	bool alreadyDetected(UserData user)
	{
		if (this->name.compare(user.name) == 0
			&& this->lastName.compare(user.lastName) == 0
			&& this->fullAddress.compare(user.fullAddress) == 0
			&& this->dateOfBirth.compare(user.dateOfBirth) == 0
			&& this->vaccinationStatus.compare(user.vaccinationStatus) == 0
			&& this->testStatus.compare(user.testStatus) == 0)
		{
			return true;
		}
		return false;
	}
};
*/

std::vector<std::string> split(std::string input, char delimiter) {
	int i = 0;
	int startIndex = 0, endIndex = 0;
	std::vector<std::string> strings;
	while (i <= strlen(input.c_str()))
	{
		if (input[i] == delimiter || i == strlen(input.c_str()))
		{
			endIndex = i;
			std::string subStr = "";
			subStr.append(input, startIndex, endIndex - startIndex);
			strings.push_back(subStr);
			startIndex = endIndex + 1;
		}
		i++;
	}

	return strings;
}

uint64_t timeSinceEpochMillisec() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t previousMillis = 0;
void caputrePhotoFromWebcam(int camIndex, cv::String pictureName) {
	cv::VideoCapture camera(camIndex);
	if (!camera.isOpened()) {
		std::cerr << "ERROR: Could not open camera" << std::endl;
		return;
	}
	cv::Mat frame;
	int currentSecond = 1;
	const long interval = 1000;
	auto currentMillis = timeSinceEpochMillisec();
	while (1) {
		camera >> frame;
		if (currentMillis - previousMillis >= interval) {
			previousMillis = currentMillis;

			currentSecond++;
		}
		cv::putText(frame,
			std::to_string(currentSecond),
			cv::Point(frame.cols / 2, frame.rows / 2),
			1,
			10,
			cv::Scalar(255, 255, 255),
			2,
			cv::LINE_8,
			false);
		cv::imshow("Camera", frame);
		currentMillis = timeSinceEpochMillisec();
		if (currentSecond >= 10) {
			camera >> frame;
			cv::imwrite(pictureName, frame);
			cv::destroyWindow("Camera");
			break;
		}
		if (cv::waitKey(27) >= 0) {
			cv::destroyWindow("Camera");
			break;
		}
	}
}

std::vector<std::string> registeredUsers;

using namespace std::chrono_literals;

void addUser(std::string data) {

	caputrePhotoFromWebcam(2, "Id.jpg");
	caputrePhotoFromWebcam(2, "face.jpg");

	SHA256 sha;
	sha.update(data);
	uint8_t* digest = sha.digest();
	std::string hash = SHA256::toString(digest);
	registeredUsers.push_back(hash);
	delete[] digest;
}

int main()
{

	cv::VideoCapture cap(0);
	if (!cap.isOpened())
		return -1;
	cv::Mat frame;
	bool scanning = true;
	bool waitingForId = true;

	while (scanning)
	{
		cap >> frame;
		cv::imshow("Camera", frame);
		if (cv::waitKey(27) >= 0)
			break;
		std::string data;
		cv::QRCodeDetector qrDecoder = cv::QRCodeDetector::QRCodeDetector();
		data = qrDecoder.detectAndDecode(frame);
		if (data.length() > 0) {
			std::vector<std::string> userInfo = split(data, '\n');
			for (int k = 0; k < userInfo.size(); k++) {
				std::cout << userInfo[k];
			}

			if (registeredUsers.size() <= 0) {
				addUser(data);
			}
			else {

				SHA256 sha;
				sha.update(data);
				uint8_t* digest = sha.digest();
				std::string hash = SHA256::toString(digest);

				for (int i = 0; i < registeredUsers.size(); i++) {
					if (registeredUsers[i].compare(hash)) {
						addUser(data);
					}
					else {
						std::cout << "User already exists\n" << std::endl;
					}
				}
				delete[] digest;
			}
		}
	}

	std::cout << "Finished" << std::endl;
	return 0;
}