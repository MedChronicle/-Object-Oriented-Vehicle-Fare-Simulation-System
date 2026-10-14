#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include <functional>
#include <limits>
#include <memory>
#include <map>
#include <set>
#include <numeric>
#include <iterator>
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <cstdint>
#include <cassert>

using namespace std;

class FareException : public runtime_error {
public:
    using runtime_error::runtime_error;
};

class InvalidDistanceException : public FareException {
public:
    explicit InvalidDistanceException(const string& m = "Distance cannot be negative!")
        : FareException(m) {}
};

class InvalidInputException : public FareException {
public:
    explicit InvalidInputException(const string& m) : FareException(m) {}
};

class InvalidCouponException : public FareException {
public:
    explicit InvalidCouponException(const string& m = "Invalid coupon code! Coupon will be ignored.")
        : FareException(m) {}
};

class InvalidBaggageException : public FareException {
public:
    explicit InvalidBaggageException(const string& m) : FareException(m) {}
};

class InsufficientBalanceException : public FareException {
public:
    explicit InsufficientBalanceException(const string& m) : FareException(m) {}
};

class VehicleUnavailableException : public FareException {
public:
    explicit VehicleUnavailableException(const string& m) : FareException(m) {}
};

namespace util {

string currentTime() {
    time_t now = time(nullptr);
    string s = ctime(&now);
    if (!s.empty() && s.back() == '\n') s.pop_back();
    return s;
}

int currentHour() {
    time_t now = time(nullptr);
    tm* lt = localtime(&now);
    return lt ? lt->tm_hour : 12;
}

string money(double v) {
    ostringstream o;
    o << fixed << setprecision(2) << v;
    return o.str();
}

string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)tolower(c); });
    return s;
}

string toUpper(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)toupper(c); });
    return s;
}

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string sanitize(string s) {
    for (char& c : s) if (c == '|' || c == '\n' || c == '\r') c = ' ';
    return trim(s);
}

[[noreturn]] void inputClosed() {
    cout << "\n  Input closed. Exiting.\n";
    exit(0);
}

void flushLine() { cin.ignore(numeric_limits<streamsize>::max(), '\n'); }

string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) inputClosed();
    return trim(s);
}

int readInt(const string& prompt) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v) { flushLine(); return v; }
        if (cin.eof()) inputClosed();
        cout << "  Invalid input. Try again.\n";
        cin.clear();
        flushLine();
    }
}

double readDouble(const string& prompt) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v) {
            flushLine();
            if (isfinite(v)) return v;
            cout << "  Enter a finite number.\n";
            continue;
        }
        if (cin.eof()) inputClosed();
        cout << "  Invalid input. Try again.\n";
        cin.clear();
        flushLine();
    }
}

bool readYesNo(const string& prompt) {
    string s = readLine(prompt);
    return !s.empty() && (s[0] == 'y' || s[0] == 'Y');
}

bool isAllDigits(const string& s) {
    return !s.empty() && all_of(s.begin(), s.end(), [](unsigned char c) { return isdigit(c); });
}

}

template <typename T, typename Pred>
vector<T> filterBy(const vector<T>& v, Pred p) {
    vector<T> out;
    copy_if(v.begin(), v.end(), back_inserter(out), p);
    return out;
}

template <typename T, typename Fn>
double sumBy(const vector<T>& v, Fn f) {
    double s = 0.0;
    for (const auto& x : v) s += f(x);
    return s;
}

struct Driver {
    string name;
    string licenseNumber;
    double rating;
    int    yearsExperience;
    string phone;

    Driver(const string& n = "Unassigned", const string& lic = "N/A",
           double r = 5.0, int yrs = 0, const string& ph = "N/A")
        : name(n), licenseNumber(lic), rating(r), yearsExperience(yrs), phone(ph) {}
};

struct TripRecord {
    int bookingID = 0;
    string customerName;
    string vehicleType;
    double distance = 0.0;
    double fare = 0.0;
    string timestamp;
    double baggageFee = 0.0;
    int rating = -1;
    string driverName = "Unassigned";
    string numberPlate = "N/A";
    string status = "Requested";
    string paymentMode = "Cash";
    string rideType = "One-way";
    int passengers = 1;
    double discountAmount = 0.0;
    int pointsEarned = 0;
    int pointsRedeemed = 0;
    string pickupLocation;
    string dropoffLocation;
    double cancellationFee = 0.0;
    double refundAmount = 0.0;

    string serialize() const {
        ostringstream o;
        o << fixed << setprecision(2)
          << bookingID << '|' << util::sanitize(customerName) << '|' << util::sanitize(vehicleType) << '|'
          << distance << '|' << fare << '|' << util::sanitize(timestamp) << '|' << baggageFee << '|'
          << rating << '|' << util::sanitize(driverName) << '|' << util::sanitize(numberPlate) << '|'
          << status << '|' << paymentMode << '|' << rideType << '|' << passengers << '|'
          << discountAmount << '|' << pointsEarned << '|' << pointsRedeemed << '|'
          << util::sanitize(pickupLocation) << '|' << util::sanitize(dropoffLocation) << '|'
          << cancellationFee << '|' << refundAmount;
        return o.str();
    }

    static bool deserialize(const string& line, TripRecord& t) {
        vector<string> f;
        stringstream ss(line);
        string part;
        while (getline(ss, part, '|')) f.push_back(part);
        if (f.size() != 16 && f.size() != 18 && f.size() != 20 && f.size() != 21) return false;
        try {
            t.bookingID = stoi(f[0]);
            t.customerName = f[1];
            t.vehicleType = f[2];
            t.distance = stod(f[3]);
            t.fare = stod(f[4]);
            t.timestamp = f[5];
            t.baggageFee = stod(f[6]);
            t.rating = stoi(f[7]);
            t.driverName = f[8];
            t.numberPlate = f[9];
            t.status = f[10];
            t.paymentMode = f[11];
            t.rideType = f[12];
            t.passengers = stoi(f[13]);
            t.discountAmount = stod(f[14]);
            t.pointsEarned = stoi(f[15]);
            if (f.size() == 21) {
                t.pointsRedeemed = stoi(f[16]);
                t.pickupLocation = f[17];
                t.dropoffLocation = f[18];
                t.cancellationFee = stod(f[19]);
                t.refundAmount = stod(f[20]);
            } else if (f.size() == 18) {
                t.pickupLocation = f[16];
                t.dropoffLocation = f[17];
            } else if (f.size() == 20) {
                t.pickupLocation = f[16];
                t.dropoffLocation = f[17];
                t.cancellationFee = stod(f[18]);
                t.refundAmount = stod(f[19]);
            } else if (t.status == "Cancelled") {
                t.cancellationFee = t.fare;
            }
        } catch (...) {
            return false;
        }
        const set<string> statuses = {"Requested", "Assigned", "In Progress", "Completed", "Cancelled"};
        bool valid = t.bookingID > 0 && !t.customerName.empty() && !t.vehicleType.empty() &&
                     isfinite(t.distance) && t.distance >= 0 && isfinite(t.fare) && t.fare >= 0 &&
                     isfinite(t.baggageFee) && t.baggageFee >= 0 && isfinite(t.discountAmount) &&
                     isfinite(t.cancellationFee) && t.cancellationFee >= 0 && isfinite(t.refundAmount) &&
                     t.refundAmount >= 0 && t.passengers > 0 && t.rating >= -1 && t.rating <= 5 &&
                     t.pointsRedeemed >= 0 && statuses.count(t.status) > 0;
        if (!valid) return false;
        if (t.status == "Cancelled")
            return t.cancellationFee <= t.fare + 0.01 && t.refundAmount <= t.fare + 0.01 &&
                   fabs(t.cancellationFee + t.refundAmount - t.fare) <= 0.02;
        return t.cancellationFee == 0.0 && t.refundAmount == 0.0;
    }
};

bool canTransitionTrip(const string& from, const string& to) {
    if (from == "Requested") return to == "Assigned" || to == "Cancelled";
    if (from == "Assigned") return to == "In Progress" || to == "Cancelled";
    if (from == "In Progress") return to == "Completed";
    return false;
}

double cancellationFeeFor(double fare) {
    if (!isfinite(fare) || fare < 0) throw InvalidInputException("Fare must be finite and non-negative.");
    return min(fare, max(20.0, fare * 0.10));
}

ostream& operator<<(ostream& os, const TripRecord& t) {
    os << fixed << setprecision(2)
       << "#" << t.bookingID << " | " << t.customerName << " | " << t.vehicleType
       << " | " << t.rideType << " | " << t.distance << " km | Rs." << t.fare
       << " | " << t.status << " | Driver: " << t.driverName;
    if (!t.pickupLocation.empty() || !t.dropoffLocation.empty())
        os << " | " << t.pickupLocation << " -> " << t.dropoffLocation;
    if (t.status == "Cancelled")
        os << " | Fee: Rs." << t.cancellationFee << " | Refund: Rs." << t.refundAmount;
    return os;
}

class Vehicle {
protected:
    int    vehicleID;
    string vehicleName;
    double ratePerKm;
    double minimumFare;
    bool   hasAC;
    string fuelType;
    string vehicleColor;
    string numberPlate;
    bool   hasMusicSystem;
    bool   hasLuggageCarrier;
    int    doorCount;
    int    seatBelts;
    int    airbags;
    double mileageKmPerLitre;
    Driver assignedDriver;

    bool   inMaintenance = false;
    bool   driverBusy = false;
    int    tripsServed   = 0;
    double revenue       = 0.0;

    static int    totalVehicles;
    static double totalEarnings;

    double slabFare(double dist, double includedKm) const {
        if (dist < 0) throw InvalidDistanceException();
        double fare = (dist <= includedKm) ? minimumFare
                                           : minimumFare + (dist - includedKm) * ratePerKm;
        return max(fare, minimumFare);
    }

public:
    Vehicle(int id, const string& name, double rate, double minFare,
            bool ac, const string& fuel, const string& color,
            const string& plate, bool music, bool luggageCarrier,
            int doors, int belts, int bags, double mileage,
            const Driver& driver)
        : vehicleID(id), vehicleName(name), ratePerKm(rate), minimumFare(minFare),
          hasAC(ac), fuelType(fuel), vehicleColor(color), numberPlate(plate),
          hasMusicSystem(music), hasLuggageCarrier(luggageCarrier),
          doorCount(doors), seatBelts(belts), airbags(bags),
          mileageKmPerLitre(mileage), assignedDriver(driver) {
        ++totalVehicles;
        cout << "  [CREATED] Vehicle object: " << vehicleName
             << "  (Plate: " << numberPlate << ")\n";
    }

    virtual ~Vehicle() {
        cout << "  [DESTROYED] Vehicle object: " << vehicleName << "\n";
    }

    virtual double calculateFare(double distanceKm) = 0;
    virtual void   displayDetails() const = 0;
    virtual bool   allowsBaggage() const { return true; }
    virtual int    freeBagAllowance() const { return 0; }
    virtual int    capacity() const { return 4; }
    virtual double avgSpeedKmh() const { return 35.0; }

    virtual void displayFullSpecs() const {
        cout << "  ----- FULL SPECIFICATION: " << vehicleName << " -----\n";
        cout << "  Plate No.        : " << numberPlate << "\n";
        cout << "  Colour           : " << vehicleColor << "\n";
        cout << "  Fuel Type        : " << fuelType << "\n";
        cout << "  Seat Capacity    : " << capacity() << " passenger(s)\n";
        cout << "  Air Conditioning : " << (hasAC ? "Available" : "Not Available") << "\n";
        cout << "  Music System     : " << (hasMusicSystem ? "Available" : "Not Available") << "\n";
        cout << "  Luggage Carrier  : " << (hasLuggageCarrier ? "Available" : "Not Available") << "\n";
        cout << "  Doors            : " << doorCount << "\n";
        cout << "  Seat Belts       : " << seatBelts << "\n";
        cout << "  Airbags          : " << airbags << "\n";
        cout << fixed << setprecision(1)
             << "  Mileage          : " << mileageKmPerLitre << " km/l\n"
             << "  Avg City Speed   : " << avgSpeedKmh() << " km/h\n";
        cout << "  Driver           : " << assignedDriver.name
             << " (" << assignedDriver.yearsExperience << " yrs exp, "
             << assignedDriver.rating << "/5)\n";
        cout << "  Driver Phone     : " << assignedDriver.phone << "\n";
        cout << "  Driver License   : " << assignedDriver.licenseNumber << "\n";
        cout << "  Status           : " << (inMaintenance ? "IN MAINTENANCE" : driverBusy ? "Driver on active trip" : "Available") << "\n";
        cout << fixed << setprecision(2)
             << "  Trips Served     : " << tripsServed << "  (Revenue Rs." << revenue << ")\n";
        cout << "  ---------------------------------------------\n";
    }

    double estimateMinutes(double distanceKm) const {
        double speed = avgSpeedKmh();
        return speed > 0 ? (distanceKm / speed) * 60.0 : 0.0;
    }

    double operatingCost(double distanceKm) const {
        if (fuelType == "Electric") return distanceKm * 1.5;
        if (mileageKmPerLitre <= 0.0) return 0.0;
        double price = 100.0;
        if (fuelType == "Petrol") price = 105.0;
        else if (fuelType == "Diesel") price = 92.0;
        else if (fuelType == "CNG") price = 78.0;
        return (distanceKm / mileageKmPerLitre) * price;
    }

    string getVehicleName()  const { return vehicleName; }
    int    getVehicleID()    const { return vehicleID; }
    double getRatePerKm()    const { return ratePerKm; }
    double getMinimumFare()  const { return minimumFare; }
    bool   getHasAC()        const { return hasAC; }
    string getFuelType()     const { return fuelType; }
    string getNumberPlate()  const { return numberPlate; }
    string getDriverName()   const { return assignedDriver.name; }
    double getDriverRating() const { return assignedDriver.rating; }
    bool   getHasMusicSystem()    const { return hasMusicSystem; }
    bool   getHasLuggageCarrier() const { return hasLuggageCarrier; }

    bool isAvailable() const { return !inMaintenance && !driverBusy; }
    bool isInMaintenance() const { return inMaintenance; }
    bool isDriverBusy() const { return driverBusy; }
    void setDriverBusy(bool value) { driverBusy = value; }
    void setMaintenance(bool m) { inMaintenance = m; }
    int    getTripsServed() const { return tripsServed; }
    double getRevenue()     const { return revenue; }

    void recordTrip(double amount) { ++tripsServed; revenue += amount; totalEarnings += amount; }
    void adjustRevenue(double delta) { revenue += delta; totalEarnings += delta; }

    static int    getTotalVehicles() { return totalVehicles; }
    static double getTotalEarnings() { return totalEarnings; }

    friend void compareFares(Vehicle& v1, Vehicle& v2, double distanceKm);
};

int    Vehicle::totalVehicles = 0;
double Vehicle::totalEarnings = 0.0;

void compareFares(Vehicle& v1, Vehicle& v2, double distanceKm) {
    if (distanceKm < 0)
        throw InvalidDistanceException("Distance for comparison cannot be negative!");

    double fare1 = v1.calculateFare(distanceKm);
    double fare2 = v2.calculateFare(distanceKm);

    cout << "\n  ===== FARE COMPARISON  (" << distanceKm << " km) =====\n";
    cout << fixed << setprecision(2);
    cout << "  " << v1.vehicleName << "  [AC:" << (v1.hasAC ? "Y" : "N")
         << " Fuel:" << v1.fuelType << " Seats:" << v1.capacity() << "]  :  Rs." << fare1
         << "  (ETA " << setprecision(0) << v1.estimateMinutes(distanceKm) << " min)\n";
    cout << fixed << setprecision(2);
    cout << "  " << v2.vehicleName << "  [AC:" << (v2.hasAC ? "Y" : "N")
         << " Fuel:" << v2.fuelType << " Seats:" << v2.capacity() << "]  :  Rs." << fare2
         << "  (ETA " << setprecision(0) << v2.estimateMinutes(distanceKm) << " min)\n";
    cout << fixed << setprecision(2);
    if (fare1 < fare2)
        cout << "  >> " << v1.vehicleName << " is cheaper by Rs." << (fare2 - fare1) << "\n";
    else if (fare2 < fare1)
        cout << "  >> " << v2.vehicleName << " is cheaper by Rs." << (fare1 - fare2) << "\n";
    else
        cout << "  >> Both vehicles charge the same fare!\n";
    cout << "  ================================================\n";
}

class Car : public Vehicle {
public:
    Car() : Vehicle(1, "Car", 15.0, 80.0, true, "Petrol", "White", "RJ-01-AB-1234",
                    true, true, 4, 5, 6, 16.5,
                    Driver("Ramesh Kumar", "RJ0120230011234", 4.6, 6, "9876543210")) {}

    double calculateFare(double dist) override {
        if (dist < 0) throw InvalidDistanceException();
        double fare;
        if      (dist <= 5)  fare = minimumFare;
        else if (dist <= 15) fare = minimumFare + (dist - 5) * ratePerKm;
        else                 fare = minimumFare + 10 * ratePerKm + (dist - 15) * 12.0;
        if (hasAC) fare += 10.0;
        return max(fare, minimumFare);
    }

    void displayDetails() const override {
        cout << "  [CAR]        Rate: Rs.15/km | Min: Rs.80 | AC: Yes (+Rs.10) | Fuel: Petrol"
             << " | Slabs: 0-5(min), 5-15(Rs.15), 15+(Rs.12) | Seats: 4\n";
    }
    int capacity() const override { return 4; }
    double avgSpeedKmh() const override { return 38.0; }
};

class Bus : public Vehicle {
    int seatingCapacity;
public:
    Bus() : Vehicle(2, "Bus", 5.0, 30.0, false, "Diesel", "Blue", "RJ-02-BC-5678",
                    false, true, 2, 40, 0, 6.0,
                    Driver("Suresh Yadav", "RJ0120180022345", 4.3, 12, "9876500001")),
            seatingCapacity(40) {}

    double calculateFare(double dist) override { return slabFare(dist, 10); }

    void displayDetails() const override {
        cout << "  [BUS]        Rate: Rs.5/km  | Min: Rs.30 | AC: No | Fuel: Diesel"
             << " | Capacity: " << seatingCapacity << " seats\n";
    }
    int capacity() const override { return seatingCapacity; }
    double avgSpeedKmh() const override { return 25.0; }
};

class AC_Bus : public Vehicle {
    int seatingCapacity;
public:
    AC_Bus() : Vehicle(10, "AC Bus", 7.5, 50.0, true, "Diesel", "Silver", "RJ-10-BD-1010",
                       true, true, 2, 35, 0, 5.5,
                       Driver("Mahesh Chand", "RJ0120170033456", 4.5, 15, "9876500002")),
               seatingCapacity(35) {}

    double calculateFare(double dist) override { return slabFare(dist, 10); }

    void displayDetails() const override {
        cout << "  [AC BUS]     Rate: Rs.7.5/km| Min: Rs.50 | AC: Yes | Fuel: Diesel"
             << " | Capacity: " << seatingCapacity << " seats\n";
    }
    int capacity() const override { return seatingCapacity; }
    double avgSpeedKmh() const override { return 28.0; }
};

class Bike : public Vehicle {
public:
    Bike() : Vehicle(3, "Bike", 8.0, 40.0, false, "Petrol", "Black", "RJ-03-CD-9012",
                     false, false, 0, 1, 0, 45.0,
                     Driver("Vikram Singh", "RJ0120220044567", 4.7, 3, "9876500003")) {}

    double calculateFare(double dist) override { return slabFare(dist, 3); }

    void displayDetails() const override {
        cout << "  [BIKE]       Rate: Rs.8/km  | Min: Rs.40 | AC: No | Fuel: Petrol"
             << " | Slabs: 0-3(min), 3+(Rs.8) | Seats: 1\n";
    }
    bool allowsBaggage() const override { return false; }
    int capacity() const override { return 1; }
    double avgSpeedKmh() const override { return 42.0; }
};

class Auto : public Vehicle {
public:
    Auto() : Vehicle(4, "Auto", 10.0, 50.0, false, "CNG", "Yellow-Green", "RJ-04-DE-3456",
                     false, true, 3, 4, 0, 22.0,
                     Driver("Ganesh Prasad", "RJ0120190055678", 4.2, 9, "9876500004")) {}

    double calculateFare(double dist) override { return slabFare(dist, 4); }

    void displayDetails() const override {
        cout << "  [AUTO]       Rate: Rs.10/km | Min: Rs.50 | AC: No | Fuel: CNG"
             << " | Slabs: 0-4(min), 4+(Rs.10) | Seats: 3\n";
    }
    int capacity() const override { return 3; }
    double avgSpeedKmh() const override { return 30.0; }
};

class LuxuryCar : public Vehicle {
    string carModel;
public:
    LuxuryCar() : Vehicle(5, "Luxury Car", 25.0, 200.0, true, "Petrol", "Jet Black", "RJ-05-EF-7890",
                          true, true, 4, 5, 8, 12.0,
                          Driver("Arjun Mehta", "RJ0120150066789", 4.9, 18, "9876500005")),
                  carModel("Premium Sedan") {}

    double calculateFare(double dist) override {
        if (dist < 0) throw InvalidDistanceException();
        return max(minimumFare + dist * ratePerKm, minimumFare);
    }

    void displayDetails() const override {
        cout << "  [LUXURY CAR] Rate: Rs.25/km | Min: Rs.200 | AC: Yes | Fuel: Petrol"
             << " | Model: " << carModel << " | 1 free bag | Seats: 4\n";
    }
    int freeBagAllowance() const override { return 1; }
    int capacity() const override { return 4; }
    double avgSpeedKmh() const override { return 45.0; }
};

class SUV : public Vehicle {
    int seatingCapacity;
public:
    SUV() : Vehicle(6, "SUV", 20.0, 150.0, true, "Diesel", "Grey", "RJ-06-FG-2345",
                    true, true, 5, 7, 6, 14.0,
                    Driver("Deepak Rathore", "RJ0120160077890", 4.4, 11, "9876500006")),
            seatingCapacity(7) {}

    double calculateFare(double dist) override { return slabFare(dist, 6); }

    void displayDetails() const override {
        cout << "  [SUV]        Rate: Rs.20/km | Min: Rs.150 | AC: Yes | Fuel: Diesel"
             << " | Seats: " << seatingCapacity << " | 1 free bag\n";
    }
    int freeBagAllowance() const override { return 1; }
    int capacity() const override { return seatingCapacity; }
    double avgSpeedKmh() const override { return 40.0; }
};

class MiniVan : public Vehicle {
    int seatingCapacity;
public:
    MiniVan() : Vehicle(7, "MiniVan", 18.0, 120.0, true, "Diesel", "Beige", "RJ-07-GH-6789",
                        true, true, 4, 8, 4, 15.5,
                        Driver("Rakesh Joshi", "RJ0120170088901", 4.1, 10, "9876500007")),
                seatingCapacity(8) {}

    double calculateFare(double dist) override { return slabFare(dist, 8); }

    void displayDetails() const override {
        cout << "  [MINIVAN]    Rate: Rs.18/km | Min: Rs.120 | AC: Yes | Fuel: Diesel"
             << " | Seats: " << seatingCapacity << "\n";
    }
    int capacity() const override { return seatingCapacity; }
    double avgSpeedKmh() const override { return 36.0; }
};

class Truck : public Vehicle {
    double capacityTons;
public:
    Truck() : Vehicle(8, "Truck", 22.0, 250.0, false, "Diesel", "Red", "RJ-08-HI-0123",
                      false, false, 2, 2, 2, 8.0,
                      Driver("Bhagwan Das", "RJ0120140099012", 4.0, 20, "9876500008")),
              capacityTons(1.5) {}

    double calculateFare(double dist) override {
        if (dist < 0) throw InvalidDistanceException();
        return max(minimumFare + dist * ratePerKm, minimumFare);
    }

    void displayDetails() const override {
        cout << "  [TRUCK]      Rate: Rs.22/km | Min: Rs.250 | AC: No | Fuel: Diesel"
             << " | Capacity: " << capacityTons << " tons | 1 free bag | Seats: 2\n";
    }
    int freeBagAllowance() const override { return 1; }
    int capacity() const override { return 2; }
    double avgSpeedKmh() const override { return 30.0; }
};

class Bicycle : public Vehicle {
public:
    Bicycle() : Vehicle(9, "Bicycle", 3.0, 10.0, false, "None", "Green", "N/A",
                        false, false, 0, 0, 0, 0.0,
                        Driver("Self-Ride", "N/A", 5.0, 0, "N/A")) {}

    double calculateFare(double dist) override { return slabFare(dist, 2); }

    void displayDetails() const override {
        cout << "  [BICYCLE]    Rate: Rs.3/km  | Min: Rs.10 | AC: No | Fuel: None"
             << " | Eco-friendly | No baggage allowed | Seats: 1\n";
    }
    bool allowsBaggage() const override { return false; }
    int capacity() const override { return 1; }
    double avgSpeedKmh() const override { return 12.0; }
};

class ElectricCar : public Vehicle {
    double batteryRangeKm;
public:
    ElectricCar() : Vehicle(11, "Electric Car", 17.0, 100.0, true, "Electric", "Pearl White", "RJ-11-EV-4321",
                            true, true, 4, 5, 6, 0.0,
                            Driver("Sanjay Verma", "RJ0120210110123", 4.8, 4, "9876500009")),
                    batteryRangeKm(320.0) {}

    double calculateFare(double dist) override {
        if (dist > batteryRangeKm)
            throw InvalidDistanceException("Distance exceeds the Electric Car's battery range!");
        return slabFare(dist, 5);
    }

    void displayDetails() const override {
        cout << "  [ELECTRIC]   Rate: Rs.17/km | Min: Rs.100 | AC: Yes | Fuel: Electric"
             << " | Range: " << fixed << setprecision(0) << batteryRangeKm
             << " km | 1 free bag | Seats: 4\n";
    }
    int freeBagAllowance() const override { return 1; }
    int capacity() const override { return 4; }
    double avgSpeedKmh() const override { return 38.0; }
};

class Customer {
    string name;
    string customerType;
    string couponCode;
    string phoneNumber;
    string preferredPayment;
    vector<TripRecord> tripHistory;
    double wallet        = 0.0;
    int    loyaltyPoints = 0;
    set<string> usedCoupons;

    static int totalCustomers;

public:
    Customer(const string& n, const string& type = "regular",
             const string& phone = "N/A", const string& payment = "Cash", bool announce = true)
        : name(util::sanitize(n)), customerType(type), couponCode(""),
          phoneNumber(phone), preferredPayment(payment) {
        ++totalCustomers;
        if (announce) cout << "  [REGISTERED] Customer: " << name << " (" << customerType << ")\n";
    }

    ~Customer() { cout << "  [SESSION ENDED] Customer: " << name << "\n"; }

    void setCoupon(const string& code)      { couponCode = util::toUpper(util::sanitize(code)); }
    void setPhone(const string& phone)      { phoneNumber = phone; }
    void setPaymentMethod(const string& m)  { preferredPayment = m; }

    string getName()          const { return name; }
    string getType()          const { return customerType; }
    string getCoupon()        const { return couponCode; }
    string getPhone()         const { return phoneNumber; }
    string getPaymentMethod() const { return preferredPayment; }
    const vector<TripRecord>& getHistory() const { return tripHistory; }

    void addTrip(const TripRecord& tr) { tripHistory.push_back(tr); }

    void updateTrip(const TripRecord& tr) {
        auto it = find_if(tripHistory.begin(), tripHistory.end(), [&](const TripRecord& x) { return x.bookingID == tr.bookingID; });
        if (it != tripHistory.end()) *it = tr;
        else tripHistory.push_back(tr);
    }

    TripRecord* lastCancellableTrip() {
        for (auto it = tripHistory.rbegin(); it != tripHistory.rend(); ++it)
            if (it->status == "Requested" || it->status == "Assigned") return &*it;
        return nullptr;
    }

    double totalSpent() const {
        return sumBy(tripHistory, [](const TripRecord& t) { return t.status == "Completed" ? t.fare : 0.0; });
    }

    string serializeProfile() const {
        string used;
        for (const auto& code : usedCoupons) {
            if (!used.empty()) used += ',';
            used += code;
        }
        if (used.empty()) used = "-";
        ostringstream o;
        o << util::sanitize(name) << '|' << customerType << '|' << util::sanitize(couponCode) << '|'
          << util::sanitize(phoneNumber) << '|' << preferredPayment << '|' << fixed << setprecision(2)
          << wallet << '|' << loyaltyPoints << '|' << used;
        return o.str();
    }

    static shared_ptr<Customer> fromProfile(const string& line) {
        vector<string> f;
        stringstream ss(line);
        string part;
        while (getline(ss, part, '|')) f.push_back(part);
        if (f.size() != 8 || f[0].empty()) return nullptr;
        try {
            double savedWallet = stod(f[5]);
            int savedPoints = stoi(f[6]);
            if (!isfinite(savedWallet) || savedWallet < 0 || savedPoints < 0) return nullptr;
            string payment = f[4];
            if (payment != "Cash" && payment != "Card" && payment != "UPI" && payment != "Wallet") payment = "Cash";
            string type = f[1];
            if (type != "regular" && type != "student" && type != "senior" && type != "member") type = "regular";
            auto c = make_shared<Customer>(f[0], type, f[3], payment, false);
            c->couponCode = f[2] == "-" ? "" : f[2];
            c->wallet = savedWallet;
            c->loyaltyPoints = savedPoints;
            if (f[7] != "-") {
                stringstream cs(f[7]);
                string code;
                while (getline(cs, code, ',')) if (!code.empty()) c->usedCoupons.insert(code);
            }
            return c;
        } catch (...) {
            return nullptr;
        }
    }

    double getWallet() const { return wallet; }
    void topUp(double amt) {
        if (!isfinite(amt) || amt <= 0) throw InvalidInputException("Top-up amount must be a finite positive value!");
        if (amt > 10000) throw InvalidInputException("Maximum single top-up is Rs.10000!");
        wallet += amt;
    }
    void credit(double amt) { if (isfinite(amt) && amt > 0) wallet += amt; }
    void pay(double amt) {
        if (!isfinite(amt) || amt < 0) throw InvalidInputException("Payment amount must be finite and non-negative.");
        if (amt > wallet)
            throw InsufficientBalanceException("Insufficient wallet balance! Available: Rs." + util::money(wallet));
        wallet -= amt;
    }

    int  getPoints() const { return loyaltyPoints; }
    void addPoints(int p)  { loyaltyPoints += p; }
    void removePoints(int p) { loyaltyPoints = max(0, loyaltyPoints - p); }
    void redeemPoints(int p) {
        if (p < 0 || p > loyaltyPoints) throw InvalidInputException("Not enough loyalty points!");
        loyaltyPoints -= p;
    }

    string tier() const {
        double s = totalSpent();
        if (s >= 2000) return "Gold";
        if (s >= 500)  return "Silver";
        return "Bronze";
    }

    bool hasUsedCoupon(const string& c) const { return usedCoupons.count(c) > 0; }
    void markCouponUsed(const string& c)      { usedCoupons.insert(c); }

    Customer operator+(const Customer& other) const {
        Customer merged("Merged[" + name + "+" + other.name + "]", "regular");
        merged.tripHistory = tripHistory;
        merged.tripHistory.insert(merged.tripHistory.end(),
                                  other.tripHistory.begin(), other.tripHistory.end());
        return merged;
    }

    void showHistory() const {
        if (tripHistory.empty()) { cout << "  No trips yet.\n"; return; }
        cout << "\n  ===== TRIP HISTORY: " << name << " =====\n";
        for (size_t i = 0; i < tripHistory.size(); ++i) {
            const TripRecord& t = tripHistory[i];
            cout << fixed << setprecision(2)
                 << "  " << (i + 1) << ". [#" << t.bookingID << "] " << t.vehicleType
                 << " (" << t.rideType << ") | " << t.distance << " km"
                 << " | Rs." << t.fare
                 << " | Bag Fee: Rs." << t.baggageFee
                 << " | Pay: " << t.paymentMode
                 << " | " << t.status
                 << " | Driver: " << t.driverName;
            if (!t.pickupLocation.empty() || !t.dropoffLocation.empty())
                cout << " | " << t.pickupLocation << " -> " << t.dropoffLocation;
            if (t.status == "Cancelled") cout << " | Fee: Rs." << t.cancellationFee << " | Refund: Rs." << t.refundAmount;
            cout << " | " << t.timestamp;
            if (t.rating >= 1) cout << " | Rating: " << t.rating << "/5";
            else               cout << " | Rating: Not rated";
            cout << "\n";
        }
    }

    static int getTotalCustomers() { return totalCustomers; }
};

int Customer::totalCustomers = 0;

struct Coupon {
    double percent;
    int    maxUses;
    int    used;
    bool   oncePerCustomer;
    bool   active;
};

class CouponManager {
    map<string, Coupon> coupons;

    void save() const {
        const string temp = "coupons.dat.tmp";
        ofstream f(temp, ios::trunc);
        if (!f.is_open()) return;
        for (const auto& kv : coupons)
            f << kv.first << '|' << fixed << setprecision(2) << kv.second.percent << '|'
              << kv.second.maxUses << '|' << kv.second.used << '|'
              << (kv.second.oncePerCustomer ? 1 : 0) << '|' << (kv.second.active ? 1 : 0) << '\n';
        f.close();
        if (!f || std::rename(temp.c_str(), "coupons.dat") != 0) remove(temp.c_str());
    }

    void load() {
        ifstream f("coupons.dat");
        string line;
        while (getline(f, line)) {
            vector<string> fields;
            stringstream ss(line);
            string part;
            while (getline(ss, part, '|')) fields.push_back(part);
            if (fields.size() != 6) continue;
            try {
                string code = util::toUpper(fields[0]);
                double pct = stod(fields[1]);
                int maxUses = stoi(fields[2]);
                int used = stoi(fields[3]);
                int once = stoi(fields[4]);
                int active = stoi(fields[5]);
                bool safeCode = !code.empty() && all_of(code.begin(), code.end(), [](unsigned char ch) {
                    return isalnum(ch) || ch == '_' || ch == '-';
                });
                if (!safeCode || !isfinite(pct) || pct <= 0 || pct > 50 || maxUses == 0 || maxUses < -1 || used < 0)
                    continue;
                coupons[code] = Coupon{pct, maxUses, used, once != 0, active != 0};
            } catch (...) {}
        }
    }

public:
    CouponManager() {
        coupons["SAVE10"] = Coupon{10.0, -1, 0, false, true};
        coupons["FIRST50"] = Coupon{50.0, -1, 0, true, true};
        coupons["VIT25"] = Coupon{25.0, -1, 0, false, true};
        coupons["WELCOME15"] = Coupon{15.0, 100, 0, true, true};
        load();
    }

    double lookup(const string& rawCode, const Customer& c) const {
        string code = util::toUpper(rawCode);
        auto it = coupons.find(code);
        if (it == coupons.end()) throw InvalidCouponException();
        const Coupon& cp = it->second;
        if (!cp.active) throw InvalidCouponException("Coupon is no longer active.");
        if (cp.maxUses >= 0 && cp.used >= cp.maxUses)
            throw InvalidCouponException("Coupon usage limit has been reached.");
        if (cp.oncePerCustomer && c.hasUsedCoupon(code))
            throw InvalidCouponException("You have already used this coupon.");
        return cp.percent / 100.0;
    }

    void redeem(const string& rawCode, Customer& c) {
        string code = util::toUpper(rawCode);
        auto it = coupons.find(code);
        if (it == coupons.end()) return;
        ++it->second.used;
        c.markCouponUsed(code);
        save();
    }

    void add(const string& rawCode, double pct, int maxUses, bool once) {
        string code = util::toUpper(rawCode);
        if (!isfinite(pct) || pct <= 0 || pct > 50) throw InvalidInputException("Coupon percentage must be in (0, 50].");
        if (code.empty() || !all_of(code.begin(), code.end(), [](unsigned char ch) {
            return isalnum(ch) || ch == '_' || ch == '-';
        })) throw InvalidInputException("Coupon codes may contain only letters, digits, underscores, or hyphens.");
        if (maxUses == 0 || maxUses < -1) throw InvalidInputException("Max uses must be positive or -1 for unlimited.");
        coupons[code] = Coupon{pct, maxUses, 0, once, true};
        save();
    }

    bool setActive(const string& rawCode, bool state) {
        auto it = coupons.find(util::toUpper(rawCode));
        if (it == coupons.end()) return false;
        it->second.active = state;
        save();
        return true;
    }

    void list() const {
        cout << "\n  ===== COUPONS =====\n";
        for (const auto& kv : coupons) {
            const Coupon& c = kv.second;
            cout << "  " << left << setw(12) << kv.first << right
                 << fixed << setprecision(0) << setw(4) << c.percent << "% | used " << c.used << "/"
                 << (c.maxUses < 0 ? string("inf") : to_string(c.maxUses))
                 << " | " << (c.oncePerCustomer ? "once per customer" : "repeatable")
                 << " | " << (c.active ? "ACTIVE" : "DISABLED") << "\n";
        }
    }
};

class FareCalculator {
    double surgeMultiplier;

    static constexpr double SMALL_BAG_FEE   = 20.0;
    static constexpr double MEDIUM_BAG_FEE  = 40.0;
    static constexpr double LARGE_BAG_FEE   = 70.0;
    static constexpr double AC_SURCHARGE    = 15.0;
    static constexpr double MUSIC_SURCHARGE = 5.0;

public:
    FareCalculator() : surgeMultiplier(1.0) {}

    void setSurge(double m) {
        if (!isfinite(m) || m <= 0 || m > 5) throw InvalidInputException("Surge multiplier must be in (0, 5]!");
        surgeMultiplier = m;
        cout << "  [ADMIN] Surge pricing set to " << m << "x\n";
    }
    double getSurge() const { return surgeMultiplier; }

    double applyCharges(double baseFare, bool night, bool peak, int waitMins, double luggageKg) {
        if (!isfinite(baseFare) || baseFare < 0 || waitMins < 0 || !isfinite(luggageKg) || luggageKg < 0)
            throw InvalidInputException("Invalid base fare, waiting time, or luggage weight.");
        double fare = baseFare * surgeMultiplier;
        if (surgeMultiplier != 1.0) cout << "  [Surge " << surgeMultiplier << "x] Applied\n";
        if (night) { fare *= 1.20; cout << "  [Night Charge +20%] Applied\n"; }
        if (peak)  { fare *= 1.15; cout << "  [Peak Hour +15%] Applied\n"; }
        if (waitMins > 0) {
            double wc = waitMins * 2.0;
            fare += wc;
            cout << "  [Waiting Rs." << util::money(wc) << " for " << waitMins << " min] Applied\n";
        }
        if (luggageKg > 0) {
            double lc = luggageKg * 10.0;
            fare += lc;
            cout << "  [Luggage Rs." << util::money(lc) << " for " << luggageKg << " kg] Applied\n";
        }
        return fare;
    }

    double applyComfortSurcharges(double fare, bool wantAC, bool vehicleHasAC,
                                  bool wantMusic, bool vehicleHasMusic) {
        if (wantAC && vehicleHasAC) {
            fare += AC_SURCHARGE;
            cout << "  [AC Comfort Surcharge Rs." << util::money(AC_SURCHARGE) << "] Applied\n";
        } else if (wantAC) {
            cout << "  [NOTE] This vehicle has no AC. Surcharge skipped.\n";
        }
        if (wantMusic && vehicleHasMusic) {
            fare += MUSIC_SURCHARGE;
            cout << "  [Music System Surcharge Rs." << util::money(MUSIC_SURCHARGE) << "] Applied\n";
        } else if (wantMusic) {
            cout << "  [NOTE] This vehicle has no music system. Surcharge skipped.\n";
        }
        return fare;
    }

    double applyBaggageFee(int smallBags, int mediumBags, int largeBags,
                           int freeAllowance, bool& freeApplied) {
        if (smallBags < 0 || mediumBags < 0 || largeBags < 0)
            throw InvalidBaggageException("Bag counts cannot be negative!");
        if (smallBags > 100 || mediumBags > 100 || largeBags > 100)
            throw InvalidBaggageException("Each bag category is limited to 100 bags per ride.");

        double fee = smallBags * SMALL_BAG_FEE + mediumBags * MEDIUM_BAG_FEE + largeBags * LARGE_BAG_FEE;
        freeApplied = false;
        int total = smallBags + mediumBags + largeBags;
        if (freeAllowance > 0 && total > 0) {
            if      (largeBags > 0)  { fee -= LARGE_BAG_FEE;  freeApplied = true; }
            else if (mediumBags > 0) { fee -= MEDIUM_BAG_FEE; freeApplied = true; }
            else if (smallBags > 0)  { fee -= SMALL_BAG_FEE;  freeApplied = true; }
        }
        return max(fee, 0.0);
    }

    double computeDiscountRate(const Customer& cust, const CouponManager& cm, string& couponUsed) {
        double disc = 0.0;
        couponUsed.clear();

        if      (cust.getType() == "student") { disc += 0.10; cout << "  [Student Discount 10%]\n"; }
        else if (cust.getType() == "senior")  { disc += 0.15; cout << "  [Senior Citizen 15%]\n"; }
        else if (cust.getType() == "member")  { disc += 0.20; cout << "  [Membership 20%]\n"; }

        string tier = cust.tier();
        if      (tier == "Gold")   { disc += 0.05; cout << "  [Gold Tier Bonus 5%]\n"; }
        else if (tier == "Silver") { disc += 0.02; cout << "  [Silver Tier Bonus 2%]\n"; }

        if (!cust.getCoupon().empty()) {
            try {
                double c = cm.lookup(cust.getCoupon(), cust);
                disc += c;
                couponUsed = cust.getCoupon();
                cout << "  [Coupon " << couponUsed << " +" << fixed << setprecision(0) << c * 100 << "%]\n";
            } catch (const InvalidCouponException& e) {
                cout << "  WARNING: " << e.what() << "\n";
            }
        }
        return min(disc, 0.50);
    }

    static double roundFare(double fare) { return round(fare); }
};

class FileHandler {
    static const string DATA_FILE;
    static const string LOG_FILE;

public:
    static void appendLog(const TripRecord& tr) {
        ofstream f(LOG_FILE, ios::app);
        if (f.is_open()) {
            f << fixed << setprecision(2)
              << "Booking #" << tr.bookingID << " | Customer: " << tr.customerName
              << " | Vehicle: " << tr.vehicleType << " (" << tr.rideType << ")"
              << " | Distance: " << tr.distance << " km"
              << " | Fare: Rs." << tr.fare
              << " | Bag Fee: Rs." << tr.baggageFee
              << " | Pay: " << tr.paymentMode
              << " | Status: " << tr.status
              << " | Pickup: " << tr.pickupLocation
              << " | Drop-off: " << tr.dropoffLocation
              << " | Driver: " << tr.driverName
              << " | Plate: " << tr.numberPlate
              << " | Cancellation fee: Rs." << tr.cancellationFee
              << " | Refund: Rs." << tr.refundAmount
              << " | Rating: " << (tr.rating >= 1 ? to_string(tr.rating) + "/5" : string("N/A"))
              << " | Time: " << tr.timestamp << "\n";
        }
    }

    static void showLog() {
        ifstream f(LOG_FILE);
        if (!f.is_open()) { cout << "  No saved trip records found.\n"; return; }
        cout << "\n  ===== ALL SAVED TRIPS =====\n";
        string line;
        while (getline(f, line)) cout << "  " << line << "\n";
    }

    static void saveAll(const vector<TripRecord>& trips) {
        const string temp = DATA_FILE + ".tmp";
        ofstream f(temp, ios::trunc);
        if (!f.is_open()) { cout << "  WARNING: could not write " << temp << "\n"; return; }
        for (const auto& t : trips) f << t.serialize() << "\n";
        f.close();
        if (!f || std::rename(temp.c_str(), DATA_FILE.c_str()) != 0) {
            remove(temp.c_str());
            cout << "  WARNING: could not safely replace " << DATA_FILE << "\n";
        }
    }

    static void saveCustomers(const vector<shared_ptr<Customer>>& customers) {
        const string fileName = "customers.dat";
        const string temp = fileName + ".tmp";
        ofstream f(temp, ios::trunc);
        if (!f.is_open()) { cout << "  WARNING: could not save customer profiles.\n"; return; }
        for (const auto& c : customers) if (c) f << c->serializeProfile() << "\n";
        f.close();
        if (!f || std::rename(temp.c_str(), fileName.c_str()) != 0) {
            remove(temp.c_str());
            cout << "  WARNING: could not safely replace customer profiles.\n";
        }
    }

    static vector<shared_ptr<Customer>> loadCustomers() {
        vector<shared_ptr<Customer>> out;
        ifstream f("customers.dat");
        string line;
        while (getline(f, line)) {
            auto c = Customer::fromProfile(line);
            if (!c) continue;
            auto it = find_if(out.begin(), out.end(), [&](const shared_ptr<Customer>& x) {
                return util::toLower(x->getName()) == util::toLower(c->getName());
            });
            if (it == out.end()) out.push_back(c);
            else *it = c;
        }
        return out;
    }

    static vector<TripRecord> loadAll() {
        vector<TripRecord> out;
        ifstream f(DATA_FILE);
        if (!f.is_open()) return out;
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            TripRecord t;
            if (TripRecord::deserialize(line, t)) out.push_back(t);
        }
        return out;
    }

    static bool exportCSV(const vector<TripRecord>& trips, const string& fileName) {
        ofstream f(fileName);
        if (!f.is_open()) return false;
        f << "BookingID,Customer,Vehicle,RideType,Passengers,Distance,Fare,BagFee,Discount,"
             "Payment,Status,Driver,Plate,Rating,Pickup,Dropoff,CancellationFee,Refund,Timestamp\n";
        auto csv = [](const string& value) {
            string out = "\"";
            for (char c : value) { if (c == '\"') out += '\"'; out += c; }
            return out + "\"";
        };
        f << fixed << setprecision(2);
        for (const auto& t : trips) {
            f << t.bookingID << ',' << csv(t.customerName) << ',' << csv(t.vehicleType) << ','
              << csv(t.rideType) << ',' << t.passengers << ',' << t.distance << ',' << t.fare << ','
              << t.baggageFee << ',' << t.discountAmount << ',' << csv(t.paymentMode) << ','
              << csv(t.status) << ',' << csv(t.driverName) << ',' << csv(t.numberPlate) << ','
              << (t.rating >= 1 ? to_string(t.rating) : string("")) << ',' << csv(t.pickupLocation) << ','
              << csv(t.dropoffLocation) << ',' << t.cancellationFee << ',' << t.refundAmount << ','
              << csv(t.timestamp) << '\n';
        }
        return true;
    }
};

const string FileHandler::DATA_FILE = "trips.dat";
const string FileHandler::LOG_FILE  = "trip_history.txt";

struct TransactionRecord {
    long long transactionID = 0;
    string timestamp;
    string customerName;
    string type;
    string paymentMode;
    double amount = 0.0;
    double balanceAfter = 0.0;
    string reference;
    string details;

    string serialize() const {
        ostringstream o;
        o << transactionID << '|' << util::sanitize(timestamp) << '|' << util::sanitize(customerName)
          << '|' << type << '|' << paymentMode << '|' << fixed << setprecision(2) << amount
          << '|' << balanceAfter << '|' << util::sanitize(reference) << '|' << util::sanitize(details);
        return o.str();
    }

    static bool deserialize(const string& line, TransactionRecord& t) {
        vector<string> f;
        stringstream ss(line);
        string part;
        while (getline(ss, part, '|')) f.push_back(part);
        if (f.size() != 9) return false;
        try {
            t.transactionID = stoll(f[0]);
            t.timestamp = f[1];
            t.customerName = f[2];
            t.type = f[3];
            t.paymentMode = f[4];
            t.amount = stod(f[5]);
            t.balanceAfter = stod(f[6]);
            t.reference = f[7];
            t.details = f[8];
        } catch (...) { return false; }
        return t.transactionID > 0 && isfinite(t.amount) && isfinite(t.balanceAfter);
    }
};

class TransactionLedger {
    vector<TransactionRecord> entries;
    long long nextID = 1;
    const string fileName = "transactions.dat";

public:
    TransactionLedger() {
        ifstream f(fileName);
        string line;
        while (getline(f, line)) {
            TransactionRecord t;
            if (TransactionRecord::deserialize(line, t)) {
                entries.push_back(t);
                nextID = max(nextID, t.transactionID + 1);
            }
        }
    }

    bool record(const string& customer, const string& type, const string& mode,
                double amount, double balance, int bookingID, const string& details) {
        if (!isfinite(amount) || !isfinite(balance)) return false;
        TransactionRecord t;
        t.transactionID = nextID;
        t.timestamp = util::currentTime();
        t.customerName = util::sanitize(customer);
        t.type = util::sanitize(type);
        t.paymentMode = util::sanitize(mode);
        t.amount = amount;
        t.balanceAfter = balance;
        t.reference = bookingID > 0 ? "BOOKING-" + to_string(bookingID) : "ACCOUNT";
        t.details = util::sanitize(details);
        ofstream f(fileName, ios::app);
        if (!f.is_open()) return false;
        f << t.serialize() << '\n';
        f.flush();
        if (!f) return false;
        entries.push_back(t);
        ++nextID;
        return true;
    }

    void show() const {
        cout << "\n  ===== TRANSACTION LEDGER =====\n";
        if (entries.empty()) { cout << "  No transactions recorded.\n"; return; }
        for (const auto& t : entries)
            cout << "  TXN-" << t.transactionID << " | " << t.timestamp << " | " << t.customerName
                 << " | " << t.type << " | " << t.paymentMode << " | Rs." << util::money(t.amount)
                 << " | Balance Rs." << util::money(t.balanceAfter) << " | " << t.reference
                 << " | " << t.details << "\n";
    }

    bool exportCSV(const string& path = "transactions_report.csv") const {
        ofstream f(path, ios::trunc);
        if (!f.is_open()) return false;
        f << "TransactionID,Timestamp,Customer,Type,PaymentMode,Amount,BalanceAfter,Reference,Details\n";
        auto csv = [](const string& value) {
            string out = "\"";
            for (char c : value) { if (c == '\"') out += '\"'; out += c; }
            return out + "\"";
        };
        for (const auto& t : entries)
            f << t.transactionID << ',' << csv(t.timestamp) << ',' << csv(t.customerName) << ','
              << csv(t.type) << ',' << csv(t.paymentMode) << ',' << fixed << setprecision(2)
              << t.amount << ',' << t.balanceAfter << ',' << csv(t.reference) << ',' << csv(t.details) << '\n';
        return static_cast<bool>(f);
    }
};

class ReceiptGenerator {
    static void row(const string& label, const string& value) {
        cout << "  " << left << setw(14) << label << ": " << value << "\n";
    }

public:
    static void print(const TripRecord& tr, double baseFare, double etaMinutes) {
        cout << "\n";
        cout << "  +------------------------------------------+\n";
        cout << "  |     SMART VEHICLE FARE  -  RECEIPT        |\n";
        cout << "  +------------------------------------------+\n";
        row("Booking ID",   "#" + to_string(tr.bookingID));
        row("Customer",     tr.customerName);
        row("Vehicle",      tr.vehicleType + " (" + tr.rideType + ")");
        row("Pickup",       tr.pickupLocation.empty() ? "Not specified" : tr.pickupLocation);
        row("Drop-off",     tr.dropoffLocation.empty() ? "Not specified" : tr.dropoffLocation);
        row("Driver",       tr.driverName);
        row("Plate No.",    tr.numberPlate);
        row("Status",       tr.status);
        row("Passengers",   to_string(tr.passengers));
        row("Distance",     util::money(tr.distance) + " km");
        row("Est. Time",    to_string((int)round(etaMinutes)) + " min");
        row("Base Fare",    "Rs." + util::money(baseFare));
        row("Baggage Fee",  "Rs." + util::money(tr.baggageFee));
        row("Discounts",    "-Rs." + util::money(tr.discountAmount));
        row("Final Fare",   "Rs." + util::money(tr.fare));
        if (tr.passengers > 1)
            row("Per Person", "Rs." + util::money(tr.fare / tr.passengers));
        row("Payment Mode", tr.paymentMode);
        row("Points Earned", to_string(tr.pointsEarned));
        row("Date & Time",  tr.timestamp);
        cout << "  +------------------------------------------+\n";
        cout << "  | Booking recorded by SmartFare.            |\n";
        cout << "  +------------------------------------------+\n\n";
    }
};

class AdminPanel {
    FareCalculator&                  calc;
    vector<TripRecord>&              trips;
    CouponManager&                   coupons;
    vector<unique_ptr<Vehicle>>&     fleet;
    vector<shared_ptr<Customer>>&    customers;
    TransactionLedger&               ledger;
    string                           password;

    Vehicle* findVehicle(const string& name) const {
        for (const auto& v : fleet) if (v->getVehicleName() == name) return v.get();
        return nullptr;
    }

    void ratingsSummary() const {
        cout << "\n  ===== VEHICLE RATINGS SUMMARY =====\n";
        for (const auto& v : fleet) {
            string vt = v->getVehicleName();
            auto rated = filterBy(trips, [&vt](const TripRecord& t) {
                return t.vehicleType == vt && t.status == "Completed" && t.rating >= 1;
            });
            cout << "  " << left << setw(14) << vt << ": ";
            if (!rated.empty()) {
                double avg = sumBy(rated, [](const TripRecord& t) { return (double)t.rating; }) / rated.size();
                cout << fixed << setprecision(2) << avg << " / 5  (" << rated.size() << " rating(s))\n";
            } else {
                cout << "No ratings yet\n";
            }
        }
    }

    void driverLeaderboard() const {
        struct DStat { int trips = 0; double earn = 0; double ratingSum = 0; int ratings = 0; string plate; };
        map<string, DStat> stats;
        for (const auto& t : trips) {
            if (t.status != "Completed") continue;
            DStat& d = stats[t.driverName];
            ++d.trips; d.earn += t.fare; d.plate = t.numberPlate;
            if (t.rating >= 1) { d.ratingSum += t.rating; ++d.ratings; }
        }
        if (stats.empty()) { cout << "  No driver activity recorded yet.\n"; return; }

        vector<pair<string, DStat>> sorted(stats.begin(), stats.end());
        sort(sorted.begin(), sorted.end(),
             [](const auto& a, const auto& b) { return a.second.earn > b.second.earn; });

        cout << "\n  ===== DRIVER LEADERBOARD (by earnings) =====\n";
        int rank = 1;
        for (const auto& kv : sorted) {
            const DStat& d = kv.second;
            cout << "  " << rank++ << ". " << left << setw(16) << kv.first
                 << " | Plate: " << setw(14) << d.plate
                 << " | Trips: " << setw(3) << d.trips
                 << " | Earned: Rs." << fixed << setprecision(2) << setw(9) << d.earn
                 << " | Avg rating: ";
            if (d.ratings) cout << (d.ratingSum / d.ratings) << "\n";
            else           cout << "N/A\n";
        }
    }

    void revenueReport() const {
        cout << "\n  ===== REVENUE & MARGIN REPORT =====\n";
        cout << "  " << left << setw(14) << "Vehicle" << right << setw(7) << "Trips"
             << setw(12) << "Revenue" << setw(12) << "Fuel Cost" << setw(12) << "Margin" << "\n";
        double totRev = 0, totCost = 0;
        for (const auto& v : fleet) {
            string vt = v->getVehicleName();
            auto done = filterBy(trips, [&vt](const TripRecord& t) { return t.vehicleType == vt; });
            double rev = sumBy(done, [](const TripRecord& t) {
                if (t.status == "Completed") return t.fare;
                if (t.status == "Cancelled") return t.cancellationFee;
                return 0.0;
            });
            double cost = sumBy(done, [&v](const TripRecord& t) {
                return t.status == "Completed" ? v->operatingCost(t.distance) : 0.0;
            });
            size_t completed = count_if(done.begin(), done.end(), [](const TripRecord& t) { return t.status == "Completed"; });
            totRev += rev; totCost += cost;
            cout << "  " << left << setw(14) << vt << right << setw(7) << completed
                 << fixed << setprecision(2) << setw(12) << rev << setw(12) << cost
                 << setw(12) << (rev - cost) << "\n";
        }
        cout << "  " << string(57, '-') << "\n";
        cout << "  " << left << setw(14) << "TOTAL" << right << setw(7) << trips.size()
             << fixed << setprecision(2) << setw(12) << totRev << setw(12) << totCost
             << setw(12) << (totRev - totCost) << "\n";
    }

    void topCustomers() const {
        map<string, double> spend;
        for (const auto& t : trips) if (t.status == "Completed") spend[t.customerName] += t.fare;
        if (spend.empty()) { cout << "  No customer activity yet.\n"; return; }
        vector<pair<string, double>> v(spend.begin(), spend.end());
        sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
        cout << "\n  ===== TOP CUSTOMERS =====\n";
        for (size_t i = 0; i < v.size() && i < 5; ++i)
            cout << "  " << (i + 1) << ". " << left << setw(20) << v[i].first
                 << " Rs." << fixed << setprecision(2) << v[i].second << "\n";
    }

    void couponMenu() {
        coupons.list();
        cout << "  1. Add coupon   2. Enable/Disable coupon   3. Back\n";
        int c = util::readInt("  Choice: ");
        try {
            if (c == 1) {
                string code = util::toUpper(util::sanitize(util::readLine("  New coupon code: ")));
                double pct  = util::readDouble("  Discount % (1-50): ");
                int maxUses = util::readInt("  Max total uses (-1 = unlimited): ");
                bool once   = util::readYesNo("  Once per customer? (y/n): ");
                coupons.add(code, pct, maxUses < 0 ? -1 : maxUses, once);
                cout << "  Coupon " << code << " created.\n";
            } else if (c == 2) {
                string code = util::toUpper(util::readLine("  Coupon code: "));
                bool state  = util::readYesNo("  Enable it? (y = enable, n = disable): ");
                if (coupons.setActive(code, state)) cout << "  Coupon updated.\n";
                else cout << "  Coupon not found.\n";
            }
        } catch (const FareException& e) {
            cout << "  ERROR: " << e.what() << "\n";
        }
    }

    void maintenanceMenu() {
        cout << "\n  ===== VEHICLE STATUS =====\n";
        for (size_t i = 0; i < fleet.size(); ++i)
            cout << "  " << (i + 1) << ". " << left << setw(14) << fleet[i]->getVehicleName()
                 << (fleet[i]->isInMaintenance() ? "IN MAINTENANCE" : fleet[i]->isDriverBusy() ? "DRIVER BUSY" : "Available") << "\n";
        int n = util::readInt("  Toggle maintenance for vehicle (0 to cancel): ");
        if (n < 1 || n > (int)fleet.size()) return;
        Vehicle* v = fleet[n - 1].get();
        if (v->isDriverBusy() && !v->isInMaintenance()) {
            cout << "  Cannot place this vehicle in maintenance while its driver has an active trip.\n";
            return;
        }
        v->setMaintenance(!v->isInMaintenance());
        cout << "  " << v->getVehicleName() << " maintenance mode is "
             << (v->isInMaintenance() ? "ON" : "OFF") << ".\n";
    }

    Customer* findCustomer(const string& name) const {
        for (const auto& c : customers)
            if (c && util::toLower(c->getName()) == util::toLower(name)) return c.get();
        return nullptr;
    }

    TripRecord* findTrip(int id) const {
        auto it = find_if(trips.begin(), trips.end(), [id](const TripRecord& t) { return t.bookingID == id; });
        return it == trips.end() ? nullptr : &*it;
    }

    void persistTrip(const TripRecord& trip) {
        for (const auto& c : customers)
            if (c && util::toLower(c->getName()) == util::toLower(trip.customerName)) c->updateTrip(trip);
        FileHandler::appendLog(trip);
        FileHandler::saveAll(trips);
        FileHandler::saveCustomers(customers);
    }

    void listLifecycleTrips() const {
        cout << "\n  ===== TRIP DISPATCH QUEUE =====\n";
        if (trips.empty()) { cout << "  No bookings yet.\n"; return; }
        for (const auto& t : trips) cout << "  " << t << "\n";
    }

    void tripLifecycleMenu() {
        int choice = 0;
        do {
            cout << "\n  ===== TRIP DISPATCH & LIFECYCLE =====\n"
                 << "  1. View all bookings\n"
                 << "  2. Assign driver to requested booking\n"
                 << "  3. Start assigned trip\n"
                 << "  4. Complete in-progress trip\n"
                 << "  5. Cancel requested/assigned trip\n"
                 << "  6. Back\n";
            choice = util::readInt("  Choice: ");
            if (choice == 1) {
                listLifecycleTrips();
                continue;
            }
            if (choice < 2 || choice > 5) continue;
            int id = util::readInt("  Booking ID: ");
            TripRecord* t = findTrip(id);
            if (!t) { cout << "  Booking not found.\n"; continue; }
            if (choice == 2) {
                if (t->status != "Requested" || !canTransitionTrip(t->status, "Assigned")) {
                    cout << "  Only requested bookings can be assigned.\n";
                    continue;
                }
                Vehicle* v = findVehicle(t->vehicleType);
                if (!v || !v->isAvailable()) {
                    cout << "  The selected vehicle/driver is unavailable.\n";
                    continue;
                }
                t->driverName = v->getDriverName();
                t->status = "Assigned";
                v->setDriverBusy(true);
                persistTrip(*t);
                ledger.record(t->customerName, "DRIVER_ASSIGNED", "N/A", 0.0, 0.0, t->bookingID,
                              "Driver " + t->driverName + " assigned to " + t->vehicleType);
                cout << "  Driver " << t->driverName << " assigned to booking #" << t->bookingID << ".\n";
            } else if (choice == 3) {
                if (!canTransitionTrip(t->status, "In Progress")) {
                    cout << "  Only assigned trips can be started.\n";
                    continue;
                }
                t->status = "In Progress";
                persistTrip(*t);
                ledger.record(t->customerName, "TRIP_STARTED", "N/A", 0.0, 0.0, t->bookingID, "Trip started");
                cout << "  Booking #" << t->bookingID << " is now In Progress.\n";
            } else if (choice == 4) {
                if (!canTransitionTrip(t->status, "Completed")) {
                    cout << "  Only in-progress trips can be completed.\n";
                    continue;
                }
                int rating = util::readInt("  Passenger rating (1-5, 0 to skip): ");
                t->rating = rating >= 1 && rating <= 5 ? rating : -1;
                t->status = "Completed";
                t->pointsEarned = static_cast<int>(floor(t->fare / 10.0));
                Customer* c = findCustomer(t->customerName);
                if (c) c->addPoints(t->pointsEarned);
                if (Vehicle* v = findVehicle(t->vehicleType)) {
                    v->recordTrip(t->fare);
                    v->setDriverBusy(false);
                }
                persistTrip(*t);
                ledger.record(t->customerName, "TRIP_COMPLETED", "N/A", 0.0, c ? c->getWallet() : 0.0,
                              t->bookingID, "Loyalty points awarded: " + to_string(t->pointsEarned));
                cout << "  Booking #" << t->bookingID << " completed. Loyalty points awarded: "
                     << t->pointsEarned << ".\n";
            } else if (choice == 5) {
                if (t->status != "Requested" && t->status != "Assigned") {
                    cout << "  Only requested or assigned trips can be cancelled.\n";
                    continue;
                }
                if (!util::readYesNo("  Confirm cancellation? (y/n): ")) {
                    cout << "  Cancellation aborted.\n";
                    continue;
                }
                const string previousStatus = t->status;
                double fee = cancellationFeeFor(t->fare);
                double refund = max(0.0, t->fare - fee);
                Customer* c = findCustomer(t->customerName);
                if (c) {
                    c->credit(refund);
                    c->addPoints(t->pointsRedeemed);
                }
                t->status = "Cancelled";
                t->cancellationFee = fee;
                t->refundAmount = refund;
                t->pointsEarned = 0;
                if (Vehicle* v = findVehicle(t->vehicleType)) {
                    if (previousStatus == "Assigned") v->setDriverBusy(false);
                    v->adjustRevenue(fee);
                }
                persistTrip(*t);
                if (refund > 0) ledger.record(t->customerName, "WALLET_REFUND", "Wallet", refund,
                                             c ? c->getWallet() : 0.0, t->bookingID, "Cancellation refund");
                ledger.record(t->customerName, "CANCELLATION_FEE", t->paymentMode, fee,
                              c ? c->getWallet() : 0.0, t->bookingID, "Pre-trip cancellation fee");
                cout << "  Booking #" << t->bookingID << " cancelled. Fee Rs." << util::money(fee)
                     << ", wallet refund Rs." << util::money(refund) << ".\n";
            }
        } while (choice != 6);
    }

    void transactionMenu() {
        int c;
        do {
            cout << "\n  ===== FINANCIAL LEDGER =====\n"
                 << "  1. View transactions\n"
                 << "  2. Export transactions to CSV\n"
                 << "  3. Back\n";
            c = util::readInt("  Choice: ");
            if (c == 1) ledger.show();
            else if (c == 2) {
                if (ledger.exportCSV()) cout << "  Exported to transactions_report.csv.\n";
                else cout << "  Could not export transactions.\n";
            }
        } while (c != 3);
    }

public:
    AdminPanel(FareCalculator& c, vector<TripRecord>& t, CouponManager& cm,
               vector<unique_ptr<Vehicle>>& f, vector<shared_ptr<Customer>>& cu,
               TransactionLedger& tl, const string& pwd = "admin123")
        : calc(c), trips(t), coupons(cm), fleet(f), customers(cu), ledger(tl), password(pwd) {}

    void run() {
        bool ok = false;
        for (int attempt = 1; attempt <= 3 && !ok; ++attempt) {
            string pwd = util::readLine("  Enter admin password: ");
            if (pwd == password) ok = true;
            else cout << "  Wrong password (" << attempt << "/3).\n";
        }
        if (!ok) { cout << "  Access denied!\n"; return; }

        int ch;
        do {
            cout << "\n  ===== ADMIN PANEL =====\n"
                 << "  1.  Totals (vehicles / customers / earnings)\n"
                 << "  2.  View Trip Log File\n"
                 << "  3.  Set Surge Multiplier\n"
                 << "  4.  Vehicle Ratings Summary\n"
                 << "  5.  Driver Leaderboard\n"
                 << "  6.  Revenue & Margin Report\n"
                 << "  7.  Top Customers\n"
                 << "  8.  Coupon Manager\n"
                 << "  9.  Vehicle Maintenance Mode\n"
                 << "  10. Export Trips to CSV\n"
                 << "  11. Change Admin Password\n"
                 << "  12. Exit Admin\n"
                 << "  13. Trip Dispatch & Lifecycle\n"
                 << "  14. Transaction Ledger\n";
            ch = util::readInt("  Choice: ");

            switch (ch) {
            case 1:
                cout << "  Total Vehicles : " << Vehicle::getTotalVehicles() << "\n"
                     << "  Total Customers: " << Customer::getTotalCustomers()
                     << " (active this session: " << customers.size() << ")\n"
                     << "  Total Earnings : Rs." << util::money(Vehicle::getTotalEarnings()) << "\n"
                     << "  Total Bookings : " << trips.size() << "\n"
                     << "  Surge Now      : " << calc.getSurge() << "x\n";
                break;
            case 2:  FileHandler::showLog(); break;
            case 3: {
                double m = util::readDouble("  Enter surge multiplier (e.g. 1.5): ");
                try { calc.setSurge(m); }
                catch (const FareException& e) { cout << "  ERROR: " << e.what() << "\n"; }
                break;
            }
            case 4:  ratingsSummary(); break;
            case 5:  driverLeaderboard(); break;
            case 6:  revenueReport(); break;
            case 7:  topCustomers(); break;
            case 8:  couponMenu(); break;
            case 9:  maintenanceMenu(); break;
            case 10:
                if (FileHandler::exportCSV(trips, "trips_report.csv"))
                    cout << "  Exported " << trips.size() << " trip(s) to trips_report.csv\n";
                else
                    cout << "  ERROR: could not write trips_report.csv\n";
                break;
            case 11: {
                string np = util::readLine("  New password (min 6 chars): ");
                if (np.size() < 6) cout << "  Password too short. Unchanged.\n";
                else { password = np; cout << "  Password updated.\n"; }
                break;
            }
            case 12: cout << "  Exiting admin panel.\n"; break;
            case 13: tripLifecycleMenu(); break;
            case 14: transactionMenu(); break;
            default: cout << "  Invalid choice.\n";
            }
        } while (ch != 12);
    }
};

class SmartFareApp {
    vector<unique_ptr<Vehicle>>   fleet;
    vector<shared_ptr<Customer>>  customers;
    vector<TripRecord>            allTrips;
    FareCalculator                calc;
    CouponManager                 coupons;
    TransactionLedger             ledger;
    AdminPanel                    admin;
    shared_ptr<Customer>          current;
    int                           nextBookingID = 1001;

    Vehicle* findVehicle(const string& name) const {
        for (const auto& v : fleet) if (v->getVehicleName() == name) return v.get();
        return nullptr;
    }

    void listVehicles() const {
        cout << "\n  ===== AVAILABLE VEHICLES =====\n";
        for (size_t i = 0; i < fleet.size(); ++i) {
            cout << "  " << (i + 1) << ". ";
            fleet[i]->displayDetails();
            if (fleet[i]->isInMaintenance()) cout << "       ^ currently in maintenance\n";
            else if (fleet[i]->isDriverBusy()) cout << "       ^ driver is on an active trip\n";
        }
    }

    int pickVehicleIndex(const string& prompt) const {
        int n = util::readInt(prompt + " (1-" + to_string(fleet.size()) + "): ");
        if (n < 1 || n > (int)fleet.size())
            throw InvalidInputException("Vehicle number is out of range!");
        return n - 1;
    }

    static string normalisePayment(const string& raw) {
        string p = util::toLower(raw);
        if (p == "card")   return "Card";
        if (p == "upi")    return "UPI";
        if (p == "wallet") return "Wallet";
        return "Cash";
    }

    void printTrips(const vector<TripRecord>& list, const string& heading) const {
        cout << "\n  ===== " << heading << " =====\n";
        if (list.empty()) { cout << "  No matching trips.\n"; return; }
        for (const auto& t : list) cout << "  " << t << "\n";
    }

    void registerCustomer() {
        string name = util::sanitize(util::readLine("  Enter name: "));
        if (name.empty()) { cout << "  Name cannot be empty.\n"; return; }

        for (const auto& c : customers) {
            if (util::toLower(c->getName()) == util::toLower(name)) {
                current = c;
                cout << "  Welcome back, " << c->getName() << "!  Tier: " << c->tier()
                     << " | Points: " << c->getPoints()
                     << " | Wallet: Rs." << util::money(c->getWallet()) << "\n";
                return;
            }
        }

        string phone = util::readLine("  Phone number (10 digits): ");
        if (phone.size() != 10 || !util::isAllDigits(phone)) {
            cout << "  Invalid phone number - stored as N/A.\n";
            phone = "N/A";
        }
        string type = util::toLower(util::readLine("  Customer type [regular / student / senior / member]: "));
        if (type != "student" && type != "senior" && type != "member") type = "regular";
        string payment = normalisePayment(util::readLine("  Preferred payment [Cash / Card / UPI / Wallet]: "));

        current = make_shared<Customer>(name, type, phone, payment);
        customers.push_back(current);

        int restored = 0;
        for (const auto& t : allTrips)
            if (util::toLower(t.customerName) == util::toLower(name)) { current->addTrip(t); ++restored; }
        if (restored) cout << "  Restored " << restored << " past trip(s) from saved records.\n";

        string coupon = util::toUpper(util::readLine("  Coupon code (leave blank to skip): "));
        if (!coupon.empty()) current->setCoupon(coupon);
        FileHandler::saveCustomers(customers);
    }

    void bookRide() {
        if (!current) { cout << "  Please register a customer first!\n"; return; }
        try {
            listVehicles();
            int idx = pickVehicleIndex("  Select vehicle");
            Vehicle* chosen = fleet[idx].get();
            if (!chosen->isAvailable())
                throw VehicleUnavailableException(chosen->getVehicleName() + " or its driver is unavailable.");

            string pickup = util::sanitize(util::readLine("  Pickup location: "));
            string dropoff = util::sanitize(util::readLine("  Destination: "));
            if (pickup.empty() || dropoff.empty()) throw InvalidInputException("Pickup and destination are required.");
            if (util::toLower(pickup) == util::toLower(dropoff))
                throw InvalidInputException("Pickup and destination must be different.");

            double dist = util::readDouble("  Estimated road distance (km): ");
            if (dist <= 0 || dist > 5000) throw InvalidDistanceException("Distance must be greater than 0 and at most 5000 km.");

            int pax = util::readInt("  Number of passengers (max " + to_string(chosen->capacity()) + "): ");
            if (pax < 1 || pax > chosen->capacity())
                throw InvalidInputException("This vehicle carries 1 to " + to_string(chosen->capacity()) + " passenger(s)!");

            bool roundTrip = util::readYesNo("  Round trip? (y/n) [10% round-trip discount]: ");
            double effectiveDist = roundTrip ? dist * 2.0 : dist;
            bool night = false, peak = false;
            if (util::readYesNo("  Auto-detect night/peak from current time? (y/n): ")) {
                int h = util::currentHour();
                night = h >= 22 || h < 5;
                peak = (h >= 8 && h < 10) || (h >= 17 && h < 20);
                cout << "  Detected hour " << h << ":00 -> night=" << (night ? "yes" : "no")
                     << ", peak=" << (peak ? "yes" : "no") << "\n";
            } else {
                night = util::readYesNo("  Night time ride? (y/n): ");
                peak = util::readYesNo("  Peak hour?       (y/n): ");
            }
            bool wantAC = util::readYesNo("  Request AC comfort surcharge? (y/n): ");
            bool wantMusic = util::readYesNo("  Request music system?         (y/n): ");
            int waitMins = util::readInt("  Waiting minutes (0 if none): ");
            double luggageKg = util::readDouble("  Luggage kg (0 if none): ");
            if (waitMins < 0 || waitMins > 1440) throw InvalidInputException("Waiting time must be between 0 and 1440 minutes.");
            if (luggageKg < 0 || luggageKg > 500) throw InvalidInputException("Luggage weight must be between 0 and 500 kg.");

            int smallBags = 0, mediumBags = 0, largeBags = 0;
            double baggageFee = 0.0;
            bool freeApplied = false;
            if (chosen->allowsBaggage()) {
                smallBags = util::readInt("  Small bags to check-in  (0 if none): ");
                mediumBags = util::readInt("  Medium bags to check-in (0 if none): ");
                largeBags = util::readInt("  Large bags to check-in  (0 if none): ");
                baggageFee = calc.applyBaggageFee(smallBags, mediumBags, largeBags, chosen->freeBagAllowance(), freeApplied);
            } else {
                cout << "  NOTE: " << chosen->getVehicleName() << " does not support baggage check-in.\n";
            }

            double baseFare = chosen->calculateFare(effectiveDist);
            cout << "\n  ===== FARE BREAKDOWN =====\n  Base Fare : Rs." << util::money(baseFare);
            if (roundTrip) {
                baseFare *= 0.90;
                cout << " -> round-trip discount 10% -> Rs." << util::money(baseFare);
            }
            cout << "\n";
            double charged = calc.applyCharges(baseFare, night, peak, waitMins, luggageKg);
            charged = calc.applyComfortSurcharges(charged, wantAC, chosen->getHasAC(), wantMusic, chosen->getHasMusicSystem());
            int totalBags = smallBags + mediumBags + largeBags;
            if (totalBags > 0) {
                cout << "  [Baggage Check-in] " << totalBags << " bag(s) -> Rs." << util::money(baggageFee);
                if (freeApplied) cout << " (1 bag free)";
                cout << "\n";
            }
            charged += baggageFee;

            string couponUsed;
            double rate = calc.computeDiscountRate(*current, coupons, couponUsed);
            double afterDisc = charged * (1.0 - rate);
            double discountAmount = charged - afterDisc;
            int pointsUsed = 0;
            if (current->getPoints() >= 100) {
                int maxByFare = static_cast<int>(floor(afterDisc * 0.20 / 0.10));
                int usable = min(current->getPoints(), maxByFare);
                if (usable >= 100 && util::readYesNo("  Redeem " + to_string(usable) + " loyalty points for Rs." +
                                                     util::money(usable * 0.10) + " off? (y/n): ")) {
                    pointsUsed = usable;
                    afterDisc -= usable * 0.10;
                    discountAmount += usable * 0.10;
                    cout << "  [Loyalty Points -Rs." << util::money(usable * 0.10) << "]\n";
                }
            }

            double finalFare = FareCalculator::roundFare(max(afterDisc, 0.0));
            double eta = chosen->estimateMinutes(effectiveDist);
            cout << "\n  ESTIMATE: Rs." << util::money(finalFare) << " | ETA ~" << static_cast<int>(round(eta))
                 << " min | Designated driver: " << chosen->getDriverName()
                 << " (" << util::money(chosen->getDriverRating()) << "/5; assigned after dispatch)\n"
                 << "  Route: " << pickup << " -> " << dropoff << "\n";
            if (!util::readYesNo("  Confirm booking request? (y/n): ")) {
                cout << "  Booking abandoned - no charge applied.\n";
                return;
            }

            string payMode = current->getPaymentMethod();
            if (payMode == "Wallet") {
                try {
                    current->pay(finalFare);
                    cout << "  [Wallet] Rs." << util::money(finalFare) << " deducted. Balance: Rs."
                         << util::money(current->getWallet()) << "\n";
                } catch (const InsufficientBalanceException& e) {
                    cout << "  WARNING: " << e.what() << "\n  Falling back to Cash payment.\n";
                    payMode = "Cash";
                }
            }
            if (pointsUsed > 0) current->redeemPoints(pointsUsed);
            if (!couponUsed.empty()) coupons.redeem(couponUsed, *current);

            TripRecord tr;
            tr.bookingID = nextBookingID++;
            tr.customerName = current->getName();
            tr.vehicleType = chosen->getVehicleName();
            tr.distance = effectiveDist;
            tr.fare = finalFare;
            tr.timestamp = util::currentTime();
            tr.baggageFee = baggageFee;
            tr.driverName = "Unassigned";
            tr.numberPlate = chosen->getNumberPlate();
            tr.status = "Requested";
            tr.paymentMode = payMode;
            tr.rideType = roundTrip ? "Round-trip" : "One-way";
            tr.passengers = pax;
            tr.discountAmount = discountAmount;
            tr.pointsEarned = 0;
            tr.pointsRedeemed = pointsUsed;
            tr.pickupLocation = pickup;
            tr.dropoffLocation = dropoff;

            ReceiptGenerator::print(tr, baseFare, eta);
            current->addTrip(tr);
            allTrips.push_back(tr);
            FileHandler::appendLog(tr);
            FileHandler::saveAll(allTrips);
            FileHandler::saveCustomers(customers);
            ledger.record(current->getName(), payMode == "Wallet" ? "WALLET_RIDE_DEBIT" : "RIDE_PAYMENT",
                          payMode, finalFare, current->getWallet(), tr.bookingID,
                          "Payment/charge recorded for " + pickup + " to " + dropoff);
            cout << "  Booking request #" << tr.bookingID << " saved. An administrator must assign a driver.\n"
                 << "  Redeemed points: " << pointsUsed << " | Remaining points: " << current->getPoints() << "\n";
        } catch (const FareException& e) {
            cout << "  [EXCEPTION] " << e.what() << "\n";
        } catch (const exception& e) {
            cout << "  [ERROR] " << e.what() << "\n";
        }
    }

    void showHistory() {
        if (!current) { cout << "  No customer registered.\n"; return; }
        current->showHistory();
        cout << "  Total Spent: Rs." << util::money(current->totalSpent())
             << "  | Tier: " << current->tier() << "\n";
    }

    void compareMenu() {
        try {
            double dist = util::readDouble("  Distance for comparison (km): ");
            int a = pickVehicleIndex("  First vehicle ");
            int b = pickVehicleIndex("  Second vehicle");
            compareFares(*fleet[a], *fleet[b], dist);
        } catch (const exception& e) {
            cout << "  [EXCEPTION] " << e.what() << "\n";
        }
    }

    void analyticsMenu() {
        cout << "\n  ===== TRIP ANALYTICS (all customers) =====\n"
             << "  1. Trips with fare above a threshold\n"
             << "  2. Top 5 highest fares\n"
             << "  3. Longest trips first\n"
             << "  4. Filter by vehicle type\n"
             << "  5. Summary statistics\n"
             << "  6. Back\n";
        int c = util::readInt("  Choice: ");
        switch (c) {
        case 1: {
            double threshold = util::readDouble("  Show trips with fare > Rs.: ");
            auto res = filterBy(allTrips, [threshold](const TripRecord& t) { return t.fare > threshold; });
            printTrips(res, "TRIPS WITH FARE > Rs." + util::money(threshold));
            break;
        }
        case 2: {
            vector<TripRecord> v = filterBy(allTrips, [](const TripRecord& t) { return t.status == "Completed"; });
            sort(v.begin(), v.end(), [](const TripRecord& a, const TripRecord& b) { return a.fare > b.fare; });
            if (v.size() > 5) v.resize(5);
            printTrips(v, "TOP 5 FARES");
            break;
        }
        case 3: {
            vector<TripRecord> v = allTrips;
            sort(v.begin(), v.end(), [](const TripRecord& a, const TripRecord& b) { return a.distance > b.distance; });
            printTrips(v, "LONGEST TRIPS");
            break;
        }
        case 4: {
            string vt = util::toLower(util::readLine("  Vehicle type (e.g. Car, SUV): "));
            auto res = filterBy(allTrips, [&vt](const TripRecord& t) { return util::toLower(t.vehicleType) == vt; });
            printTrips(res, "TRIPS FOR '" + vt + "'");
            break;
        }
        case 5: {
            auto done = filterBy(allTrips, [](const TripRecord& t) { return t.status == "Completed"; });
            if (done.empty()) { cout << "  No completed trips yet.\n"; break; }
            double total = sumBy(done, [](const TripRecord& t) { return t.fare; });
            double km    = sumBy(done, [](const TripRecord& t) { return t.distance; });
            auto mm = minmax_element(done.begin(), done.end(),
                                     [](const TripRecord& a, const TripRecord& b) { return a.fare < b.fare; });
            cout << "\n  ===== SUMMARY =====\n"
                 << "  Completed trips : " << done.size() << "\n"
                 << "  Cancelled trips : " << count_if(allTrips.begin(), allTrips.end(), [](const TripRecord& t) { return t.status == "Cancelled"; }) << "\n"
                 << "  Active bookings : " << count_if(allTrips.begin(), allTrips.end(), [](const TripRecord& t) { return t.status == "Requested" || t.status == "Assigned" || t.status == "In Progress"; }) << "\n"
                 << "  Total revenue   : Rs." << util::money(total) << "\n"
                 << "  Average fare    : Rs." << util::money(total / done.size()) << "\n"
                 << "  Revenue per km  : Rs." << util::money(km > 0 ? total / km : 0) << "\n"
                 << "  Cheapest trip   : " << *mm.first << "\n"
                 << "  Costliest trip  : " << *mm.second << "\n";
            break;
        }
        default: break;
        }
    }

    void mergeCustomers() {
        if (customers.size() < 2) { cout << "  Need at least 2 registered customers.\n"; return; }
        cout << "  Registered customers:\n";
        for (size_t i = 0; i < customers.size(); ++i)
            cout << "  " << (i + 1) << ". " << customers[i]->getName() << "\n";
        int c1 = util::readInt("  Select first customer:  ");
        int c2 = util::readInt("  Select second customer: ");
        if (c1 < 1 || c2 < 1 || c1 > (int)customers.size() || c2 > (int)customers.size()) {
            cout << "  Invalid selection.\n"; return;
        }
        Customer merged = *customers[c1 - 1] + *customers[c2 - 1];
        merged.showHistory();
        cout << "  Combined Total Spent: Rs." << util::money(merged.totalSpent()) << "\n";
    }

    void showSpecs() {
        try {
            int idx = pickVehicleIndex("  Select vehicle for full spec");
            fleet[idx]->displayFullSpecs();
        } catch (const FareException& e) {
            cout << "  " << e.what() << "\n";
        }
    }

    void walletMenu() {
        if (!current) { cout << "  Please register a customer first!\n"; return; }
        cout << "\n  ===== WALLET & LOYALTY: " << current->getName() << " =====\n"
             << "  Wallet balance : Rs." << util::money(current->getWallet()) << "\n"
             << "  Loyalty points : " << current->getPoints() << "  (100 pts = Rs.10)\n"
             << "  Tier           : " << current->tier() << "  (Silver >= Rs.500 spent, Gold >= Rs.2000)\n"
             << "  Payment method : " << current->getPaymentMethod() << "\n"
             << "  Active coupon  : " << (current->getCoupon().empty() ? "none" : current->getCoupon()) << "\n"
             << "  1. Top up wallet\n  2. Change payment method\n  3. Apply coupon code\n  4. Back\n";
        int c = util::readInt("  Choice: ");
        try {
            if (c == 1) {
                double amt = util::readDouble("  Amount to add (Rs.): ");
                current->topUp(amt);
                ledger.record(current->getName(), "WALLET_TOP_UP", "Wallet", amt, current->getWallet(), 0, "Customer wallet top-up");
                FileHandler::saveCustomers(customers);
                cout << "  New balance: Rs." << util::money(current->getWallet()) << "\n";
            } else if (c == 2) {
                current->setPaymentMethod(normalisePayment(util::readLine("  New method [Cash / Card / UPI / Wallet]: ")));
                FileHandler::saveCustomers(customers);
                cout << "  Payment method set to " << current->getPaymentMethod() << ".\n";
            } else if (c == 3) {
                string code = util::toUpper(util::readLine("  Coupon code: "));
                if (code.empty()) { current->setCoupon(""); FileHandler::saveCustomers(customers); cout << "  Coupon cleared.\n"; return; }
                double pct = coupons.lookup(code, *current);
                current->setCoupon(code);
                FileHandler::saveCustomers(customers);
                cout << "  Coupon " << code << " saved (" << fixed << setprecision(0) << pct * 100 << "% off).\n";
            }
        } catch (const FareException& e) {
            cout << "  ERROR: " << e.what() << "\n";
        }
    }

    void suggestVehicle() {
        try {
            double dist = util::readDouble("  Trip distance (km): ");
            if (dist <= 0 || dist > 5000) throw InvalidDistanceException("Distance must be greater than 0 and at most 5000 km.");
            int pax = util::readInt("  Passengers: ");
            if (pax < 1 || pax > 40) throw InvalidInputException("Passengers must be between 1 and 40.");
            double budget = util::readDouble("  Budget in Rs. (0 = no limit): ");
            if (budget < 0) throw InvalidInputException("Budget cannot be negative.");
            bool needAC = util::readYesNo("  Must have AC? (y/n): ");

            struct Option { Vehicle* v; double fare; double eta; };
            vector<Option> opts;
            for (const auto& v : fleet) {
                if (!v->isAvailable() || v->capacity() < pax) continue;
                if (needAC && !v->getHasAC()) continue;
                double f;
                try { f = v->calculateFare(dist); } catch (const FareException&) { continue; }
                if (budget > 0 && f > budget) continue;
                opts.push_back({v.get(), f, v->estimateMinutes(dist)});
            }
            if (opts.empty()) { cout << "  No vehicle matches those requirements. Try relaxing a filter.\n"; return; }

            sort(opts.begin(), opts.end(), [](const Option& a, const Option& b) { return a.fare < b.fare; });
            auto fastest = min_element(opts.begin(), opts.end(),
                                       [](const Option& a, const Option& b) { return a.eta < b.eta; });

            cout << "\n  ===== SMART SUGGESTIONS (cheapest first) =====\n";
            for (size_t i = 0; i < opts.size() && i < 5; ++i) {
                const Option& o = opts[i];
                cout << "  " << (i + 1) << ". " << left << setw(13) << o.v->getVehicleName() << right
                     << " Rs." << setw(8) << util::money(o.fare)
                     << " | ETA " << setw(3) << (int)round(o.eta) << " min"
                     << " | Seats " << o.v->capacity()
                     << " | Driver " << util::money(o.v->getDriverRating()) << "/5";
                if (i == 0) cout << "  <- Best value";
                if (&o == &*fastest) cout << "  <- Fastest";
                cout << "\n";
            }
        } catch (const FareException& e) {
            cout << "  [EXCEPTION] " << e.what() << "\n";
        }
    }

    void cancelLastRide() {
        if (!current) { cout << "  Please register a customer first!\n"; return; }
        TripRecord* pending = current->lastCancellableTrip();
        if (!pending) { cout << "  No requested or assigned trip can currently be cancelled.\n"; return; }
        TripRecord updated = *pending;
        double fee = cancellationFeeFor(updated.fare);
        double refund = max(0.0, updated.fare - fee);
        cout << "\n  Booking to cancel: " << updated << "\n"
             << "  Cancellation fee: Rs." << util::money(fee)
             << " | Wallet refund: Rs." << util::money(refund) << "\n";
        if (!util::readYesNo("  Confirm cancellation? (y/n): ")) { cout << "  Cancellation aborted.\n"; return; }
        string previousStatus = updated.status;
        updated.status = "Cancelled";
        updated.cancellationFee = fee;
        updated.refundAmount = refund;
        updated.pointsEarned = 0;
        current->credit(refund);
        current->addPoints(pending->pointsRedeemed);
        if (Vehicle* v = findVehicle(updated.vehicleType)) {
            if (previousStatus == "Assigned") v->setDriverBusy(false);
            v->adjustRevenue(fee);
        }
        current->updateTrip(updated);
        FileHandler::appendLog(updated);
        auto it = find_if(allTrips.begin(), allTrips.end(), [&](const TripRecord& t) { return t.bookingID == updated.bookingID; });
        if (it != allTrips.end()) *it = updated;
        FileHandler::saveAll(allTrips);
        FileHandler::saveCustomers(customers);
        if (refund > 0) ledger.record(updated.customerName, "WALLET_REFUND", "Wallet", refund,
                                      current->getWallet(), updated.bookingID, "Pre-trip cancellation refund");
        ledger.record(updated.customerName, "CANCELLATION_FEE", updated.paymentMode, fee,
                      current->getWallet(), updated.bookingID, "Pre-trip cancellation fee");
        cout << "  Booking #" << updated.bookingID << " cancelled. Fee Rs." << util::money(fee)
             << ", refund Rs." << util::money(refund) << ". Wallet balance: Rs."
             << util::money(current->getWallet()) << "\n";
    }

public:
    SmartFareApp()
        : admin(calc, allTrips, coupons, fleet, customers, ledger) {
        cout << "\n  ============================================\n";
        cout << "     SMART VEHICLE FARE SYSTEM  v4.0  -  VIT 2026\n";
        cout << "  ============================================\n\n";
        cout << "  Initialising vehicles...\n";

        fleet.push_back(make_unique<Car>());
        fleet.push_back(make_unique<Bus>());
        fleet.push_back(make_unique<Bike>());
        fleet.push_back(make_unique<Auto>());
        fleet.push_back(make_unique<LuxuryCar>());
        fleet.push_back(make_unique<SUV>());
        fleet.push_back(make_unique<MiniVan>());
        fleet.push_back(make_unique<Truck>());
        fleet.push_back(make_unique<Bicycle>());
        fleet.push_back(make_unique<AC_Bus>());
        fleet.push_back(make_unique<ElectricCar>());

        allTrips = FileHandler::loadAll();
        customers = FileHandler::loadCustomers();
        for (const auto& t : allTrips) {
            nextBookingID = max(nextBookingID, t.bookingID + 1);
            auto c = find_if(customers.begin(), customers.end(), [&](const shared_ptr<Customer>& x) {
                return x && util::toLower(x->getName()) == util::toLower(t.customerName);
            });
            if (c == customers.end()) {
                auto restored = make_shared<Customer>(t.customerName, "regular", "N/A", "Cash", false);
                customers.push_back(restored);
                c = prev(customers.end());
            }
            (*c)->addTrip(t);
            if (Vehicle* v = findVehicle(t.vehicleType)) {
                if (t.status == "Completed") v->recordTrip(t.fare);
                else if (t.status == "Cancelled") v->adjustRevenue(t.cancellationFee);
                else if (t.status == "Assigned" || t.status == "In Progress") v->setDriverBusy(true);
            }
        }
        FileHandler::saveCustomers(customers);
        cout << "\n  Loaded " << allTrips.size() << " saved trip(s) and " << customers.size()
             << " customer profile(s).\n\n";
    }

    void run() {
        int choice;
        do {
            cout << "  ==================================\n";
            cout << "  MAIN MENU";
            if (current) cout << "   [" << current->getName() << " | " << current->tier() << "]";
            cout << "\n";
            cout << "  1.  Register / Switch Customer\n";
            cout << "  2.  Book a Ride\n";
            cout << "  3.  View Available Vehicles\n";
            cout << "  4.  My Trip History\n";
            cout << "  5.  Compare Two Vehicle Fares (Friend Function)\n";
            cout << "  6.  Trip Analytics (Lambdas / Templates)\n";
            cout << "  7.  Merge Two Customers (Operator +)\n";
            cout << "  8.  View Full Vehicle Specifications\n";
            cout << "  9.  Wallet, Loyalty & Coupons\n";
            cout << "  10. Smart Vehicle Suggestion\n";
            cout << "  11. Cancel My Last Ride\n";
            cout << "  12. Admin Panel (dispatch, lifecycle, ledger)\n";
            cout << "  13. Exit\n";
            cout << "  ==================================\n";
            choice = util::readInt("  Choice: ");

            switch (choice) {
            case 1:  registerCustomer(); break;
            case 2:  bookRide(); break;
            case 3:  listVehicles(); break;
            case 4:  showHistory(); break;
            case 5:  compareMenu(); break;
            case 6:  analyticsMenu(); break;
            case 7:  mergeCustomers(); break;
            case 8:  showSpecs(); break;
            case 9:  walletMenu(); break;
            case 10: suggestVehicle(); break;
            case 11: cancelLastRide(); break;
            case 12: admin.run(); break;
            case 13: cout << "  Goodbye! Thank you for using SmartFare.\n"; break;
            default: cout << "  Invalid choice. Please enter 1-13.\n";
            }
        } while (choice != 13);

        shutdown();
    }

private:
    void shutdown() {
        FileHandler::saveAll(allTrips);
        FileHandler::saveCustomers(customers);

        cout << "\n  ===== SESSION SUMMARY =====\n";
        cout << "  Total Vehicle Types   : " << Vehicle::getTotalVehicles() << "\n";
        cout << "  Total Customers       : " << Customer::getTotalCustomers() << "\n";
        cout << "  Total Bookings Stored : " << allTrips.size() << "\n";
        cout << "  Total Earnings (Rs.)  : " << util::money(Vehicle::getTotalEarnings()) << "\n";
        cout << "  ===========================\n\n";
        cout << "  Trips: trips.dat | Profiles: customers.dat | Ledger: transactions.dat\n";

        cout << "  Releasing customer objects...\n";
        current.reset();
        customers.clear();

        cout << "  Releasing vehicle objects...\n";
        fleet.clear();
    }
};

int runSelfTests() {
    int checks = 0;
    auto expect = [&](bool condition, const string& label) {
        ++checks;
        if (!condition) throw runtime_error("Self-test failed: " + label);
    };
    try {
        expect(canTransitionTrip("Requested", "Assigned"), "requested-to-assigned transition");
        expect(canTransitionTrip("Assigned", "In Progress"), "assigned-to-started transition");
        expect(canTransitionTrip("In Progress", "Completed"), "started-to-completed transition");
        expect(canTransitionTrip("Assigned", "Cancelled"), "assigned-to-cancelled transition");
        expect(!canTransitionTrip("Completed", "Cancelled"), "completed trips cannot be cancelled");
        expect(!canTransitionTrip("Requested", "Completed"), "requested trips cannot skip lifecycle states");
        expect(abs(static_cast<int>(cancellationFeeFor(100.0) * 100 - 2000)) < 1, "minimum cancellation fee");
        expect(abs(static_cast<int>(cancellationFeeFor(1000.0) * 100 - 10000)) < 1, "percentage cancellation fee");
        expect(cancellationFeeFor(0.0) == 0.0, "zero fare cancellation fee");

        TripRecord t;
        t.bookingID = 1301;
        t.customerName = "Test User";
        t.vehicleType = "Car";
        t.distance = 18.5;
        t.fare = 285.0;
        t.timestamp = "Wed Oct 14 12:00:00 2026";
        t.baggageFee = 20.0;
        t.rating = 5;
        t.driverName = "Ramesh Kumar";
        t.numberPlate = "RJ-01-AB-1234";
        t.status = "Assigned";
        t.paymentMode = "Cash";
        t.rideType = "One-way";
        t.passengers = 2;
        t.discountAmount = 15.0;
        t.pointsEarned = 0;
        t.pointsRedeemed = 100;
        t.pickupLocation = "VIT Main Gate";
        t.dropoffLocation = "Katpadi Station";
        TripRecord copy;
        expect(TripRecord::deserialize(t.serialize(), copy), "trip serialization round trip");
        expect(copy.bookingID == t.bookingID && copy.status == t.status && copy.pointsRedeemed == 100 &&
               copy.pickupLocation == t.pickupLocation && copy.dropoffLocation == t.dropoffLocation,
               "trip route and loyalty fields preserved");
        TripRecord legacy;
        string oldLine = "1001|Legacy Customer|Car|10.00|250.00|Wed Oct 14 12:00:00 2026|0.00|5|Ramesh Kumar|RJ-01-AB-1234|Completed|Cash|One-way|1|0.00|25";
        expect(TripRecord::deserialize(oldLine, legacy) && legacy.bookingID == 1001 && legacy.status == "Completed",
               "legacy 16-field trip format remains readable");

        TransactionRecord tx;
        tx.transactionID = 9;
        tx.timestamp = "Wed Oct 14 12:00:00 2026";
        tx.customerName = "Test User";
        tx.type = "WALLET_TOP_UP";
        tx.paymentMode = "Wallet";
        tx.amount = 500.0;
        tx.balanceAfter = 500.0;
        tx.reference = "ACCOUNT";
        tx.details = "Self-test";
        TransactionRecord txCopy;
        expect(TransactionRecord::deserialize(tx.serialize(), txCopy) && txCopy.transactionID == 9 &&
               txCopy.amount == 500.0 && txCopy.details == "Self-test", "transaction serialization round trip");

        Car car;
        expect(car.calculateFare(5.0) == 90.0, "car fare at five kilometres");
        bool rejected = false;
        try { car.calculateFare(-1.0); } catch (const InvalidDistanceException&) { rejected = true; }
        expect(rejected, "negative distance rejected");

        Customer profile("Self Test", "student", "1234567890", "Wallet", false);
        profile.topUp(250.0);
        profile.addPoints(120);
        profile.setCoupon("SAVE10");
        profile.markCouponUsed("SAVE10");
        auto restored = Customer::fromProfile(profile.serializeProfile());
        expect(restored && restored->getWallet() == 250.0 && restored->getPoints() == 120 &&
               restored->hasUsedCoupon("SAVE10") && restored->getCoupon() == "SAVE10",
               "customer profile persistence round trip");
        cout << "PASS: " << checks << " SmartFare self-tests completed.\n";
        return 0;
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }
}

int main(int argc, char** argv) {
    if (argc > 1 && string(argv[1]) == "--self-test") return runSelfTests();
    SmartFareApp app;
    app.run();
    return 0;
}
