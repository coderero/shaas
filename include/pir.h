#if !defined(PIR_H)
#define PIR_H

class PIR
{
private:
    int _port;
    bool movement;

public:
    void set_port(int port);
    int get_port();

    bool get_movement();

    void detect_movement();
};

#endif // PIR_H
