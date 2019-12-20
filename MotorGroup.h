class MotorGroup{
public:
	MotorGroup(int m_a, int m_b)
	{
		motor_a = m_a;
		motor_b = m_b;
	}
	
	int motor_a, motor_b;
	int motor_a_speed, motor_b_speed;
  bool idle = true;
  
	void set_speed(int as, int bs)
	{
		motor_a_speed = as;
		motor_b_speed = bs;
   if ( as > 0 || bs > 0) idle = false;
	}

  void set_idle(){ idle = true; }
  
	bool otherHigh(MotorGroup* other_group)
	{
		return other_group->motor_a_speed == 0 && other_group->motor_b_speed == 0;
	}
	
};
