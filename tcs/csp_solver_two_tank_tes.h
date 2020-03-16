/**
BSD-3-Clause
Copyright 2019 Alliance for Sustainable Energy, LLC
Redistribution and use in source and binary forms, with or without modification, are permitted provided 
that the following conditions are met :
1.	Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.
2.	Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.	Neither the name of the copyright holder nor the names of its contributors may be used to endorse 
or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER, CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES 
DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __csp_solver_two_tank_tes_
#define __csp_solver_two_tank_tes_

#include "csp_solver_core.h"
#include "csp_solver_util.h"

#include "sam_csp_util.h"

const int N_tes_pipe_sections = 11;

class C_heat_exchanger
{
private:
	HTFProperties mc_field_htfProps;
	HTFProperties mc_store_htfProps;

	double m_m_dot_des_ave;		    //[kg/s] Average (field and storage sides) mass flow rate
	double m_eff_des;				//[-] Heat exchanger effectiveness
	double m_UA_des;				//[W/K] Heat exchanger conductance

    // Stored values from previous timestep
    double m_T_hot_field_prev;		//[K] Hotter temperature on field side (opposite tank side)
    double m_T_cold_field_prev;		//[K] Colder temperature on field side (opposite tank side)
    double m_m_dot_field_prev;      //[kg/s] Mass flow rate on field side (opposite tank side)
    double m_T_hot_tes_prev;		//[K] Hotter temperature on TES side (tank side)
    double m_T_cold_tes_prev;       //[K] Colder temperature on TES side (tank side)
    double m_m_dot_tes_prev;        //[kg/s] Mass flow rate on TES side (tank side)

    // Calculated values for current timestep
    double m_T_hot_field_calc;		//[K] Hotter temperature on field side (opposite tank side)
    double m_T_cold_field_calc;		//[K] Colder temperature on field side (opposite tank side)
    double m_m_dot_field_calc;      //[kg/s] Mass flow rate on field side (opposite tank side)
    double m_T_hot_tes_calc;		//[K] Hotter temperature on TES side (tank side)
    double m_T_cold_tes_calc;       //[K] Colder temperature on TES side (tank side)
    double m_m_dot_tes_calc;        //[kg/s] Mass flow rate on TES side (tank side)

	void hx_performance(bool is_hot_side_mdot, bool is_storage_side, double T_hot_in, double m_dot_known, double T_cold_in,
		double &eff, double &T_hot_out, double &T_cold_out, double &q_trans, double &m_dot_solved);

public:

	C_heat_exchanger();

	void init(const HTFProperties &fluid_field, const HTFProperties &fluid_store, double q_transfer_des,
		double dt_des, double T_h_in_des, double T_h_out_des);

	void hx_charge_mdot_tes(double T_cold_tes, double m_dot_tes, double T_hot_field, 
		double &eff, double &T_hot_tes, double &T_cold_field, double &q_trans, double &m_dot_field);

	void hx_discharge_mdot_tes(double T_hot_tes, double m_dot_tes, double T_cold_field,
		double &eff, double &T_cold_tes, double &T_hot_field, double &q_trans, double &m_dot_field);

	void hx_charge_mdot_field(double T_hot_field, double m_dot_field, double T_cold_tes,
		double &eff, double &T_cold_field, double &T_hot_tes, double &q_trans, double &m_dot_tes);

	void hx_discharge_mdot_field(double T_cold_field, double m_dot_field, double T_hot_tes,
		double &eff, double &T_hot_field, double &T_cold_tes, double &q_trans, double &m_dot_tes);

    void converged();
};

class C_storage_tank
{
private:
	HTFProperties mc_htf;

	double m_V_total;			//[m^3] Total volume for *one temperature* tank
	double m_V_active;			//[m^3] active volume of *one temperature* tank (either cold or hot)
	double m_V_inactive;		//[m^3] INactive volume of *one temperature* tank (either cold or hot)
	double m_UA;				//[W/K] Tank loss conductance

	double m_T_htr;				//[K] Tank heater set point
	double m_max_q_htr;			//[MWt] Max tank heater capacity

	// Stored values from end of previous timestep
	double m_V_prev;		    //[m^3] Volume of storage fluid in tank
	double m_T_prev;		    //[K] Temperature of storage fluid in tank
	double m_m_prev;		    //[kg] Mass of storage fluid in tank

	// Calculated values for current timestep
	double m_V_calc;		    //[m^3] Volume of storage fluid in tank
	double m_T_calc;		    //[K] Temperature of storage fluid in tank
	double m_m_calc;		    //[kg] Mass of storage fluid in tank

public:

	C_storage_tank();

	double calc_mass_at_prev();

    double calc_cp_at_prev();

    double calc_enth_at_prev();

    double get_m_UA();

	double get_m_T_prev();

	double get_m_T_calc();

	double get_m_m_calc();

	void init(HTFProperties htf_class_in, double V_tank_one_temp, double h_tank, double h_min, double u_tank, 
		double tank_pairs, double T_htr, double max_q_htr, double V_ini, double T_ini);

	double m_dot_available(double f_unavail, double timestep);	

	void energy_balance(double timestep /*s*/, double m_dot_in, double m_dot_out, double T_in /*K*/, double T_amb /*K*/, 
		double &T_ave /*K*/, double &q_heater /*MW*/, double &q_dot_loss /*MW*/);

	void energy_balance_constant_mass(double timestep /*s*/, double m_dot_in, double T_in /*K*/, double T_amb /*K*/,
		double &T_ave /*K*/, double &q_heater /*MW*/, double &q_dot_loss /*MW*/);

	void converged();
};

class C_csp_two_tank_tes : public C_csp_tes
{
private:

	HTFProperties mc_field_htfProps;		// Instance of HTFProperties class for field HTF
	HTFProperties mc_store_htfProps;		// Instance of HTFProperties class for storage HTF
	
	C_heat_exchanger mc_hx;				

	//Storage_HX mc_hx_storage;				// Instance of Storage_HX class for heat exchanger between storage and field HTFs
	
	C_storage_tank mc_cold_tank;			// Instance of storage tank class for the cold tank
	C_storage_tank mc_hot_tank;				// Instance of storage tank class for the hot tank	

	// member string for exception messages
	std::string error_msg;

	// Timestep data
	double m_m_dot_tes_dc_max;  //[kg/s] TES discharge available from the SYSTEM (field side of HX if there is one)
	double m_m_dot_tes_ch_max;  //[kg/s] TES charge that can be sent to the SYSTEM (field side of HX if there is one)

	// Member data
	bool m_is_tes;
	double m_vol_tank;			//[m3] volume of *one temperature*, i.e. vol_tank = total cold storage = total hot storage
	double m_V_tank_active;		//[m^3] available volume (considering h_min) of *one temperature*
	double m_q_pb_design;		//[Wt] thermal power to power cycle at design
	double m_V_tank_hot_ini;	//[m^3] Initial volume in hot storage tank


    // Monotonic equation solvers
    class C_MEQ_indirect_tes_discharge : public C_monotonic_equation
    {
    private:
        C_csp_two_tank_tes *mpc_csp_two_tank_tes;
        double m_timestep;
        double m_T_amb;
        double m_T_cold_field;
        double m_m_dot_field;

    public:
        C_MEQ_indirect_tes_discharge(C_csp_two_tank_tes *pc_csp_two_tank_tes, double timestep, double T_amb,
            double T_cold_field, double m_dot_field)
        {
            mpc_csp_two_tank_tes = pc_csp_two_tank_tes;
            m_timestep = timestep;
            m_T_amb = T_amb;
            m_T_cold_field = T_cold_field;
            m_m_dot_field = m_dot_field;
        }

        virtual int operator()(double m_dot_tank /*kg/s*/, double *m_dot_bal /*-*/);
    };

    class C_MEQ_indirect_tes_charge : public C_monotonic_equation
    {
    private:
        C_csp_two_tank_tes *mpc_csp_two_tank_tes;
        double m_timestep;
        double m_T_amb;
        double m_T_hot_field;
        double m_m_dot_field;

    public:
        C_MEQ_indirect_tes_charge(C_csp_two_tank_tes *pc_csp_two_tank_tes, double timestep, double T_amb,
            double T_hot_field, double m_dot_field)
        {
            mpc_csp_two_tank_tes = pc_csp_two_tank_tes;
            m_timestep = timestep;
            m_T_amb = T_amb;
            m_T_hot_field = T_hot_field;
            m_m_dot_field = m_dot_field;
        }

        virtual int operator()(double m_dot_tank /*kg/s*/, double *m_dot_bal /*-*/);
    };

public:

	// Class to save messages for up stream classes
	C_csp_messages mc_csp_messages;

	struct S_params
	{
		int m_field_fl;
		util::matrix_t<double> m_field_fl_props;

		int m_tes_fl;
		util::matrix_t<double> m_tes_fl_props;

		bool m_is_hx;

		double m_W_dot_pc_design;   //[MWe] Design point gross power cycle output
		double m_eta_pc;            //[-] Design point power cycle thermal efficiency
		double m_solarm;			//[-] solar multiple
		double m_ts_hours;			//[hr] hours of storage at design power cycle operation		
		double m_h_tank;			//[m] tank height
		double m_u_tank;			//[W/m^2-K]
		int m_tank_pairs;			//[-]
		double m_hot_tank_Thtr;		//[C] convert to K in init()
		double m_hot_tank_max_heat;	//[MW]
		double m_cold_tank_Thtr;	//[C] convert to K in init()
		double m_cold_tank_max_heat;//[MW]
		double m_dt_hot;			//[C] Temperature difference across heat exchanger - assume hot and cold deltaTs are equal
		double m_T_field_in_des;	//[C] convert to K in init()
		double m_T_field_out_des;	//[C] convert to K in init()
        double m_dP_field_des;      //[bar] Total field pressure drop at design
		double m_T_tank_hot_ini;	//[C] Initial temperature in hot storage tank
		double m_T_tank_cold_ini;	//[C] Initial temperature in cold storage cold
		double m_h_tank_min;		//[m] Minimum allowable HTF height in storage tank
		double m_f_V_hot_ini;       //[%] Initial fraction of available volume that is hot
		double m_htf_pump_coef;		//[kW/kg/s] Pumping power to move 1 kg/s of HTF through power cycle
        double m_tes_pump_coef;		//[kW/kg/s] Pumping power to move 1 kg/s of HTF through tes loop
        double eta_pump;            //[-] Pump efficiency, for newer pumping calculations
        bool tanks_in_parallel;     //[-] Whether the tanks are in series or parallel with the solar field. Series means field htf must go through storage tanks.
        bool has_hot_tank_bypass;   //[-] True if the bypass valve causes the field htf to bypass just the hot tank and enter the cold tank before flowing back to the field.
        double T_tank_hot_inlet_min; //[C] Minimum field htf temperature that may enter the hot tank
        double V_tes_des;           //[m/s] Design-point velocity for sizing the diameters of the TES piping
        bool custom_tes_p_loss;     //[-] True if the TES piping losses should be calculated using the TES pipe lengths and minor loss coeffs, false if using the pumping loss parameters
        bool custom_tes_pipe_sizes;               //[-] True if the TES diameters and wall thicknesses parameters should be used instead of calculating them
        util::matrix_t<double> k_tes_loss_coeffs; //[-] Combined minor loss coefficients of the fittings and valves in the collection (including bypass) and generation loops in the TES 
        util::matrix_t<double> tes_diams;         //[m] Imported inner diameters for the TES piping as read from the modified output files
        util::matrix_t<double> tes_wallthicks;    //[m] Imported wall thicknesses for the TES piping as read from the modified output files
        util::matrix_t<double> tes_lengths;       //[m] Imported lengths for the TES piping as read from the modified output files
        bool calc_design_pipe_vals;               //[-] Should the HTF state be calculated at design conditions
        double pipe_rough;                        //[m] Pipe absolute roughness
        double DP_SGS;                            //[bar] Pressure drop within the steam generator



		S_params()
		{
			m_field_fl = m_tes_fl = m_tank_pairs = -1;		
			m_is_hx = true;
            tanks_in_parallel = true;
            has_hot_tank_bypass = true;
            custom_tes_p_loss = false;
            custom_tes_pipe_sizes = false;
            calc_design_pipe_vals = true;

			m_ts_hours = 0.0;		//[hr] Default to 0 so that if storage isn't defined, simulation won't crash

			m_W_dot_pc_design = m_eta_pc = m_solarm = m_h_tank = m_u_tank = m_hot_tank_Thtr = m_hot_tank_max_heat = m_cold_tank_Thtr =
				m_cold_tank_max_heat = m_dt_hot = m_T_field_in_des = m_T_field_out_des = m_dP_field_des = m_T_tank_hot_ini =
				m_T_tank_cold_ini = m_h_tank_min = m_f_V_hot_ini = m_htf_pump_coef = m_tes_pump_coef = eta_pump = T_tank_hot_inlet_min = 
                V_tes_des = pipe_rough = DP_SGS = std::numeric_limits<double>::quiet_NaN();

            k_tes_loss_coeffs.resize_fill(11, 0.);
            tes_diams.resize_fill(1, -1.);
            tes_wallthicks.resize_fill(1, -1.);
            double vals1[11] = { 0., 90., 100., 120., 0., 0., 0., 0., 80., 120., 80. };
            tes_lengths.assign(vals1, 11);
		}
	};

	S_params ms_params;

	C_csp_two_tank_tes();

	~C_csp_two_tank_tes(){};

	virtual void init(const C_csp_tes::S_csp_tes_init_inputs init_inputs);

    double pipe_vol_tot;	                     //[m^3]
    util::matrix_t<double> pipe_v_dot_rel;       //[-]
    util::matrix_t<double> pipe_diams;           //[m^3]
    util::matrix_t<double> pipe_wall_thk;        //[m]
    util::matrix_t<double> pipe_lengths;         //[m]
    util::matrix_t<double> pipe_m_dot_des;       //[kg/s]
    util::matrix_t<double> pipe_vel_des;         //[m/s]
    util::matrix_t<double> pipe_T_des;           //[C]
    util::matrix_t<double> pipe_P_des;           //[bar]
    double P_in_des;                             //[bar] Pressure at the inlet to the TES, at the field side
    
	virtual bool does_tes_exist();

	virtual double get_hot_temp();

	virtual double get_cold_temp();



    virtual double get_initial_charge_energy(); //MWh

    virtual double get_min_charge_energy(); //MWh

    virtual double get_max_charge_energy(); //MWh

    virtual double get_degradation_rate();  // s^-1

	virtual void discharge_avail_est(double T_cold_K, double step_s, double &q_dot_dc_est, double &m_dot_field_est, double &T_hot_field_est);

	virtual void charge_avail_est(double T_hot_K, double step_s, double &q_dot_ch_est, double &m_dot_field_est, double &T_cold_field_est);

	// Calculate pumping power...???
	virtual bool discharge(double timestep /*s*/, double T_amb /*K*/, double m_dot_htf_in /*kg/s*/, double T_htf_cold_in, double & T_htf_hot_out /*K*/, C_csp_tes::S_csp_tes_outputs &outputs);

	virtual void discharge_full(double timestep /*s*/, double T_amb /*K*/, double T_htf_cold_in, double & T_htf_hot_out /*K*/, double & m_dot_htf_out /*kg/s*/, C_csp_tes::S_csp_tes_outputs &outputs);

	virtual bool charge(double timestep /*s*/, double T_amb /*K*/, double m_dot_htf_in /*kg/s*/, double T_htf_hot_in, double & T_htf_cold_out /*K*/, C_csp_tes::S_csp_tes_outputs &outputs);

	virtual void charge_full(double timestep /*s*/, double T_amb /*K*/, double T_htf_hot_in /*K*/, double & T_htf_cold_out /*K*/, double & m_dot_htf_out /*kg/s*/, C_csp_tes::S_csp_tes_outputs &outputs);
	
	virtual void idle(double timestep, double T_amb, C_csp_tes::S_csp_tes_outputs &outputs);
	
	virtual void converged();

    virtual int pressure_drops(double m_dot_sf, double m_dot_pb,
        double T_sf_in, double T_sf_out, double T_pb_in, double T_pb_out, bool recirculating,
        double &P_drop_col, double &P_drop_gen);

    virtual double pumping_power(double m_dot_sf, double m_dot_pb, double m_dot_tank,
        double T_sf_in, double T_sf_out, double T_pb_in, double T_pb_out, bool recirculating);
};

class C_csp_cold_tes : public C_csp_tes //Class for cold storage based on two tank tes ARD
{
private:

	HTFProperties mc_field_htfProps;		// Instance of HTFProperties class for field HTF
	HTFProperties mc_store_htfProps;		// Instance of HTFProperties class for storage HTF

	C_heat_exchanger mc_hx;

	//Storage_HX mc_hx_storage;				// Instance of Storage_HX class for heat exchanger between storage and field HTFs

	C_storage_tank mc_cold_tank;			// Instance of storage tank class for the cold tank
	C_storage_tank mc_hot_tank;				// Instance of storage tank class for the hot tank	

											// member string for exception messages
	std::string error_msg;

	// Timestep data
	double m_m_dot_tes_dc_max;
	double m_m_dot_tes_ch_max;

	// Member data
	bool m_is_tes;
	double m_vol_tank;			//[m3] volume of *one temperature*, i.e. vol_tank = total cold storage = total hot storage
	double m_V_tank_active;		//[m^3] available volume (considering h_min) of *one temperature*
	double m_q_pb_design;		//[Wt] thermal power to power cycle at design
	double m_V_tank_hot_ini;	//[m^3] Initial volume in hot storage tank

public:

	// Class to save messages for up stream classes
	C_csp_messages mc_csp_messages;

	struct S_params
	{
		int m_field_fl;
		util::matrix_t<double> m_field_fl_props;

		int m_tes_fl;
		util::matrix_t<double> m_tes_fl_props;

		bool m_is_hx;

		double m_W_dot_pc_design;   //[MWe] Design point gross power cycle output
		double m_eta_pc_factor;     //[-] Factor accounting for Design point power cycle thermal efficiency
		double m_solarm;			//[-] solar multiple
		double m_ts_hours;			//[hr] hours of storage at design power cycle operation		
		double m_h_tank;			//[m] tank height
		double m_u_tank;			//[W/m^2-K]
		int m_tank_pairs;			//[-]
		double m_hot_tank_Thtr;		//[C] convert to K in init()
		double m_hot_tank_max_heat;	//[MW]
		double m_cold_tank_Thtr;	//[C] convert to K in init()
		double m_cold_tank_max_heat;//[MW]
		double m_dt_hot;			//[C] Temperature difference across heat exchanger - assume hot and cold deltaTs are equal
		double m_T_field_in_des;	//[C] convert to K in init()
		double m_T_field_out_des;	//[C] convert to K in init()
		double m_T_tank_hot_ini;	//[C] Initial temperature in hot storage tank
		double m_T_tank_cold_ini;	//[C] Initial temperature in cold storage cold
		double m_h_tank_min;		//[m] Minimum allowable HTF height in storage tank
		double m_f_V_hot_ini;       //[%] Initial fraction of available volume that is hot

		double m_htf_pump_coef;		//[kW/kg/s] Pumping power to move 1 kg/s of HTF through power cycle

		double dT_cw_rad;			//[degrees] Temperature change in cooling water for cold storage cooling.
		double m_dot_cw_rad;		//[kg/sec]	Mass flow of cooling water for cold storage cooling at design.
		int m_ctes_type;			//2= two tank (this model) 3=three node (other model)
		double m_dot_cw_cold;		//[kg/sec]	Mass flow of storage water between cold storage and radiative field HX.
		double m_lat;           //Latitude [degrees]

		S_params()
		{
			m_field_fl = m_tes_fl = m_tank_pairs = -1;
			m_is_hx = true;

			m_ts_hours = 0.0;		//[hr] Default to 0 so that if storage isn't defined, simulation won't crash
			m_ctes_type = 0;		// Default to <2 so that storage is not assumed in cost calculations.

			m_W_dot_pc_design = m_eta_pc_factor = m_solarm = m_h_tank = m_u_tank = m_hot_tank_Thtr = m_hot_tank_max_heat = m_cold_tank_Thtr =
				m_cold_tank_max_heat = m_dt_hot = m_T_field_in_des = m_T_field_out_des = m_T_tank_hot_ini =
				m_T_tank_cold_ini = m_h_tank_min = m_f_V_hot_ini = m_htf_pump_coef= dT_cw_rad=m_dot_cw_rad=m_dot_cw_cold=m_lat= std::numeric_limits<double>::quiet_NaN();
		}
	};

	S_params ms_params;

	C_csp_cold_tes();

	~C_csp_cold_tes() {};

	virtual void init(const C_csp_tes::S_csp_tes_init_inputs init_inputs);

	virtual bool does_tes_exist();

	virtual double get_hot_temp();

	virtual double get_cold_temp();

	virtual double get_hot_mass();

	virtual double get_cold_mass();

	virtual double get_hot_mass_prev();

	virtual double get_cold_mass_prev();

	virtual double get_physical_volume(); //m^3

	virtual double get_hot_massflow_avail(double step_s); //kg/sec

	virtual double get_cold_massflow_avail(double step_s); //kg/sec

	virtual double get_initial_charge_energy(); //MWh

	virtual double get_min_charge_energy(); //MWh

	virtual double get_max_charge_energy(); //MWh

	virtual double get_degradation_rate();  // s^-1

	virtual void discharge_avail_est(double T_cold_K, double step_s, double &q_dot_dc_est, double &m_dot_field_est, double &T_hot_field_est);

	virtual void charge_avail_est(double T_hot_K, double step_s, double &q_dot_ch_est, double &m_dot_field_est, double &T_cold_field_est);

	// Calculate pumping power...???
	virtual bool discharge(double timestep /*s*/, double T_amb /*K*/, double m_dot_htf_in /*kg/s*/, double T_htf_cold_in, double & T_htf_hot_out /*K*/, C_csp_tes::S_csp_tes_outputs &outputs);

	virtual void discharge_full(double timestep /*s*/, double T_amb /*K*/, double T_htf_cold_in, double & T_htf_hot_out /*K*/, double & m_dot_htf_out /*kg/s*/, C_csp_tes::S_csp_tes_outputs &outputs);

	virtual bool charge(double timestep /*s*/, double T_amb /*K*/, double m_dot_htf_in /*kg/s*/, double T_htf_hot_in, double & T_htf_cold_out /*K*/, C_csp_tes::S_csp_tes_outputs &outputs);
	
	virtual bool charge_discharge(double timestep /*s*/, double T_amb /*K*/, double m_dot_hot_in /*kg/s*/, double T_hot_in, double m_dot_cold_in /*kg/s*/, double T_cold_in, C_csp_tes::S_csp_tes_outputs &outputs);

	virtual bool recirculation(double timestep /*s*/, double T_amb /*K*/, double m_dot_cold_in /*kg/s*/, double T_cold_in /*K*/,  C_csp_tes::S_csp_tes_outputs &outputs);
	
	virtual void charge_full(double timestep /*s*/, double T_amb /*K*/, double T_htf_hot_in /*K*/, double & T_htf_cold_out /*K*/, double & m_dot_htf_out /*kg/s*/, C_csp_tes::S_csp_tes_outputs &outputs);

	virtual void idle(double timestep, double T_amb, C_csp_tes::S_csp_tes_outputs &outputs);

	virtual void converged();

    virtual int pressure_drops(double m_dot_sf, double m_dot_pb,
        double T_sf_in, double T_sf_out, double T_pb_in, double T_pb_out, bool recirculating,
        double &P_drop_col, double &P_drop_gen);

    virtual double pumping_power(double m_dot_sf, double m_dot_pb, double m_dot_tank,
        double T_sf_in, double T_sf_out, double T_pb_in, double T_pb_out, bool recirculating);
};


void two_tank_tes_sizing(HTFProperties &tes_htf_props, double Q_tes_des /*MWt-hr*/, double T_tes_hot /*K*/,
		double T_tes_cold /*K*/, double h_min /*m*/, double h_tank /*m*/, int tank_pairs /*-*/, double u_tank /*W/m^2-K*/,
		double & vol_one_temp_avail /*m3*/, double & vol_one_temp_total /*m3*/, double & d_tank /*m*/,
		double & q_dot_loss_des /*MWt*/  );

int size_tes_piping(double vel_dsn, util::matrix_t<double> L, double rho_avg, double m_dot_pb, double solarm,
    bool tanks_in_parallel, double &vol_tot, util::matrix_t<double> &v_dot_rel, util::matrix_t<double> &diams,
    util::matrix_t<double> &wall_thk, util::matrix_t<double> &m_dot, util::matrix_t<double> &vel, bool custom_sizes = false);

int size_tes_piping_TandP(HTFProperties &field_htf_props, double T_field_in /*K*/, double T_field_out /*K*/, double P_field_in /*Pa*/, double DP_SGS,
    const util::matrix_t<double> &L, const util::matrix_t<double> &k_tes_loss_coeffs, double pipe_rough,
    bool tanks_in_parallel, const util::matrix_t<double> &diams, const util::matrix_t<double> &vel,
    util::matrix_t<double> &TES_T_des, util::matrix_t<double> &TES_P_des, double &TES_P_in);

#endif   //__csp_solver_two_tank_tes_
