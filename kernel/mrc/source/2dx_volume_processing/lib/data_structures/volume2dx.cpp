/* 
 * @license GNU Public License
 * @author Nikhil Biyani (nikhilbiyani@gmail.com)
 * 
 */

#include <math.h>
#include <fftw3.h>
#include <random>

#include "volume2dx.hpp"

#include "miller_index.hpp"
#include "diffraction_spot.hpp"

#include "../io/mrc_io.hpp"
#include "../io/reflection_io.hpp"

#include "../symmetrization/symmetry2dx.hpp"
#include "../symmetrization/fourier_symmetrization.hpp"

#include "../utilities/angle_utilities.hpp"
#include "../utilities/filesystem.hpp"
#include "../utilities/fourier_utilities.hpp"
#include "../utilities/bead_model_generator.hpp"
#include "../utilities/number_utilities.hpp"

namespace ds = volume::data;

ds::Volume2dx::Volume2dx(int nx, int ny, int nz)
{
    _header = ds::VolumeHeader2dx(nx, ny, nz);
    _real = ds::RealSpaceData(nx, ny, nz);
    _fourier = ds::FourierSpaceData();
    _transform = volume::transforms::FourierTransformFFTW();
    _type = NONE;
}

ds::Volume2dx::Volume2dx(const VolumeHeader2dx& header)
{
    _header = ds::VolumeHeader2dx(header);
    _real = ds::RealSpaceData(nx(), ny(), nz());
    _fourier = ds::FourierSpaceData();
    _transform = volume::transforms::FourierTransformFFTW();
    _type = NONE;
}

ds::Volume2dx::Volume2dx(const Volume2dx& copy)
{
    _header = ds::VolumeHeader2dx(copy.header());
    _real = ds::RealSpaceData(copy._real);
    _fourier = ds::FourierSpaceData(copy._fourier);
    _transform = volume::transforms::FourierTransformFFTW(copy._transform);
    _type = copy._type;
}

ds::Volume2dx::~Volume2dx(){}

void ds::Volume2dx::reset(const Volume2dx& other)
{
    _header = ds::VolumeHeader2dx(other._header);
    _real.reset(other._real);
    _fourier.reset(other._fourier);
    _transform.reset(other._transform);
    _type = other._type;
}

void ds::Volume2dx::clear()
{
    _real.clear();
    _fourier.clear();
    _type = NONE;
}

ds::Volume2dx& ds::Volume2dx::operator=(const Volume2dx& rhs)
{
    reset(rhs);
    return *this;
}

ds::Volume2dx ds::Volume2dx::operator+(const Volume2dx& rhs)
{
    Volume2dx added(header());
    if(rhs.has_real())
    {
        RealSpaceData rhs_real = rhs._real;
        RealSpaceData sum = get_real()+ rhs_real;
        added.set_real(sum);
    }
    else
    {
        std::cerr << "Hey, Volumes cannot be added! No real data in memory. Please set the data?";
    }
    
    return added;
}

ds::Volume2dx ds::Volume2dx::operator*(double factor)
{
    Volume2dx modified;
    if(has_real())
    {
        modified.set_real(get_real()*factor);
    }
    else if(has_fourier())
    {
        modified.set_fourier(get_fourier()*factor);
    }
    else
    {
        std::cerr << "Hey, No factor can be multiplied to volume! No data in memory. Did you forget to set the data?";
    }
    
    return modified;
}

bool ds::Volume2dx::has_fourier() const
{
    if(_type == FOURIER || _type == BOTH) return true;
    else return false;
}

bool ds::Volume2dx::has_real() const
{
    if(_type == REAL || _type == BOTH) return true;
    else return false;
}

ds::RealSpaceData ds::Volume2dx::get_real()
{
    prepare_real();
    return _real;
}

ds::FourierSpaceData ds::Volume2dx::get_fourier()
{
    prepare_fourier();
    return _fourier;
}

void ds::Volume2dx::prepare_fourier()
{
    if(!has_fourier()) fourier_from_real();
}

void ds::Volume2dx::prepare_real()
{
    if(!has_real()) real_from_fourier();
}

void ds::Volume2dx::set_fourier(const FourierSpaceData& fourier)
{
    _fourier.reset(fourier);
    _type = FOURIER;
}

void ds::Volume2dx::set_real(const RealSpaceData& real)
{
    if(real.nx() == nx() && real.ny() == ny() && real.nz() == nz())
    {
        _real.reset(real);
        _type = REAL;
    }
    else
    {
        std::cerr << "Error while setting real volume. The expected dimension are not correct.\n"
                  << "Found: " << real.nx() << " " << real.ny() << " " << real.nz() << "\n"
                  << "Expected: " << nx() << " " << ny() << " " << nz() << "\n";
        exit(1);
    }
}

void ds::Volume2dx::fourier_from_real()
{
    std::cout << "Converting the real data to Fourier. \n";
    if(_type == REAL)
    {
        _fourier.clear();
        fftw_complex* complex_data = fftw_alloc_complex(fx()*fy()*fz());
        double* real_data = _real.get_data_for_fftw();
        _transform.RealToComplex(nx(), ny(), nz(), real_data, complex_data);
        _fourier.reset_data_from_fftw(fx(), fy(), fz(), complex_data);
        fftw_free(complex_data);
        fftw_free(real_data);
        _type = BOTH;
    }
    else if(_type == NONE) 
    {
        std::cerr << "Hey, Fourier data cannot be set! Real data not in memory. Did you forget to set the data?";
    }
}

void ds::Volume2dx::real_from_fourier()
{
    std::cout << "Converting the Fourier data to real. \n";
    if(_type == FOURIER){
        double* real_data = fftw_alloc_real(nx()*ny()*nz());
        fftw_complex* complex_data = _fourier.fftw_data(fx(), fy(), fz());
        _transform.ComplexToReal(nx(), ny(), nz(), complex_data, real_data);
        _type = BOTH;
        _real.set_from_fftw(real_data);
        fftw_free(real_data);
        fftw_free(complex_data);
    }
    else if(_type == NONE) {
        std::cerr << "Hey, Real data cannot be set! Fourier data not in memory. Did you forget to set the data?";
    }
}

std::string ds::Volume2dx::to_string() const
{
    std::string output = "";
    
    output += _header.to_string();
    output += data_string();
    
    return output;
}

std::string ds::Volume2dx::data_string() const
{
    std::string output = "";
    
    output += ":\nData Information:\n";
    if(has_real())
    {
        output += ":\tReal data in memory.\n" ;
        output += ":\t|Minimum density: " + std::to_string(_real.min()) + "\n";
        output += ":\t|Maximum density: " + std::to_string(_real.max()) + "\n";
        output += ":\t|Mean density: " + std::to_string(_real.mean()) + "\n";
        output += ":\n";
    }
    
    if(has_fourier())
    {
        MillerIndex max_index = max_resolution_spot();
        output += ":\tFourier data in memory.\n" ;
        output += ":\t|Number of spots: " + std::to_string(_fourier.spots()) + "\n";
        output += ":\t|Intensity sum: " + std::to_string(_fourier.intensity_sum()) + "\n";
        output += ":\t|Spot with maximum resolution: " + max_index.to_string() + " - " + std::to_string(resolution_at(max_index.h(), max_index.k(), max_index.l())) + " A\n";
        output += ":\n";
    }
    
    if(_type == NONE)
    {
        output += ":\tNo data in memory\n";
        output += ":\n";
    }
    
    return output;
    
}

ds::VolumeHeader2dx ds::Volume2dx::header() const {
    return _header;
}

void ds::Volume2dx::read_volume(std::string file_name, std::string format)
{
    std::cout << "Reading volume with format <"<< format << "> from file:\n\t" << file_name << "\n\n";
    if(format == "hkl")
    {
       set_fourier(volume::io::reflection::read(file_name, 1));
    }
    else if(format == "hkz")
    {
       set_fourier(volume::io::reflection::read(file_name, nz()));
    }
    else if (format == "mrc" || format == "map")
    {
        _header = VolumeHeader2dx(volume::io::mrc::get_header(file_name, format));
        RealSpaceData read_data = volume::io::mrc::get_data(file_name, nx(), ny(), nz());
        set_real(read_data);
    }
    else
    {
        std::cerr << "The read format <" << format << "> of file " << file_name << " not supported.\n";
    }
    
    std::cout << "Volume in memory!\n";
    
}

void ds::Volume2dx::read_volume(std::string file_name)
{
    std::string format = volume::utilities::filesystem::FileExtension(file_name);
    
    read_volume(file_name, format);
}

void ds::Volume2dx::write_volume(std::string file_name, std::string format)
{
    std::cout << "\nWriting volume with format <"<< format << "> to file:\n\t" << file_name << "\n\n";
    if(format == "hkl")
    {
        volume::io::reflection::write(file_name, get_fourier());
    }
    else if (format == "mrc" || format == "map")
    {
        volume::io::mrc::write_mrc_mode_2(file_name, header(), get_real(), format);
    }
    else
    {
        std::cerr << "The write format <" << format << "> of file " << file_name << " not supported.\n";
    }
}

void ds::Volume2dx::write_volume(std::string file_name)
{
    std::string format = volume::utilities::filesystem::FileExtension(file_name);
    write_volume(file_name, format);
}

void ds::Volume2dx::generate_random_densities(double fraction_to_fill)
{
    ds::RealSpaceData data(nx(), ny(), nz());
    int attempts = fraction_to_fill * data.size();
    for ( int attempt = 0; attempt < attempts; attempt++ ) 
    {
        int id = rand() % data.size();
        double density = rand() % 255;
        data.set_value_at(id, density);
    }
    
    set_real(data);
}

void ds::Volume2dx::generate_poisson_densities(double mean_density)
{
    std::cout << "Generating Poisson noise with expected mean: " << mean_density << "\n";
    std::default_random_engine generator;
    std::poisson_distribution<int> distribution(mean_density);
    
    ds::RealSpaceData data(nx(), ny(), nz());
    for(int id=0; id<data.size(); id++)
    {
        data.set_value_at(id, distribution(generator));
    }
    
    data.grey_scale();
    
    set_real(data);
}

double ds::Volume2dx::resolution_at(int h, int k, int l) const
{
    ds::MillerIndex index(h, k, l);
    return volume::utilities::fourier_utilities::GetResolution(index, _header.gamma(), _header.xlen(), _header.ylen(), _header.zlen());
}

ds::MillerIndex ds::Volume2dx::max_resolution_spot() const
{
    if(has_fourier())
    {
        MillerIndex spot;
        double maxres = 10000;
        for(FourierSpaceData::const_iterator itr=_fourier.begin(); itr!=_fourier.end(); ++itr)
        {
            MillerIndex index = (*itr).first;
            double res = resolution_at(index.h(), index.k(), index.l());
            if(res < maxres)
            {
                spot = index;
            }
        }
        return spot;
    }
    else
    {
        std::cerr << "WARNING: Can't calculate max resolution spot, Fourier data not in memory!!\n";
        return MillerIndex(0,0,0);
    }
}

double ds::Volume2dx::max_resolution() const
{
    MillerIndex index = max_resolution_spot();
    return resolution_at(index.h(), index.k(), index.l());
}

void ds::Volume2dx::rescale_energy(double energy)
{
    FourierSpaceData data = get_fourier();
    double curr_energy = data.intensity_sum();
    double factor = sqrt(energy/curr_energy);
    data.scale_amplitudes(factor);
    set_fourier(data);
}

void ds::Volume2dx::rescale_to_max_amplitude(double max_amplitude)
{
    FourierSpaceData data = get_fourier();
    double current_max = data.max_amplitude();
    double factor = max_amplitude/current_max;
    data.scale_amplitudes(factor);
    set_fourier(data);
}

void ds::Volume2dx::rescale_densities(double min, double max)
{
    RealSpaceData data = get_real();
    data.scale(min, max);
    this->set_real(data);
}

void ds::Volume2dx::grey_scale_densities()
{
    RealSpaceData data = get_real();
    data.grey_scale();
    this->set_real(data);
}

void ds::Volume2dx::symmetrize()
{
    std::cout << "Symmetrizing with symmetry: " << symmetry() << std::endl;
    volume::symmetrization::Symmetry2dx sym(_header.symmetry());
    prepare_fourier();
    volume::symmetrization::fourier_symmetrization::symmetrize(_fourier, sym);
    _type = FOURIER;
}

ds::StructureFactors ds::Volume2dx::calculate_structure_factors(int resolution_bins, double max_resolution) const
{
    double min_resolution = 0;
    
    ds::StructureFactors sf(min_resolution, 1/max_resolution, resolution_bins);
    sf.initialize_intensities(*this);
    
    return sf;
}

void ds::Volume2dx::apply_structure_factors(ds::StructureFactors sf_ref, double fraction)
{
    std::cout << "Applying structure factors to the volume.. \n";
    
    FourierSpaceData new_data;
    FourierSpaceData current_data = get_fourier();
    
    //Get the current structure factors with same resolution spacing as of reference
    ds::StructureFactors sf_current(sf_ref.min_resolution(), sf_ref.max_resolution(), sf_ref.resolution_bins());
    sf_current.initialize_intensities(*this);
    
    double sf_ref_max = sf_ref.max_intensity();
    double sf_current_max = sf_current.max_intensity();
    double max_scale = sf_current_max/sf_ref_max;
    
    for(FourierSpaceData::const_iterator itr=current_data.begin(); itr!=current_data.end(); ++itr)
    {
        //Get the data for current reflection
        MillerIndex index = (*itr).first;
        DiffractionSpot spot = (*itr).second;
        
        if ( index.h() != 0 || index.k() != 0 || index.l() != 0 ) 
        {
            double resolution = 1 / resolution_at(index.h(), index.k(), index.l());

            //Find the appropriate intensity

            double sf_ref_intensity = sf_ref.intensity_at(resolution);
            double sf_curr_intensity = sf_current.intensity_at(resolution);

            if ( sf_ref_intensity != -1 && sf_curr_intensity != -1 ) {
                double amplitude_scale = 0.0;
                if ( sf_curr_intensity != 0.0 ) amplitude_scale = sqrt(max_scale * sf_ref_intensity / sf_curr_intensity);

                double current_amplitude = spot.amplitude();
                double new_amplitude = amplitude_scale*current_amplitude;

                double scaled_amplitude = new_amplitude * fraction + current_amplitude * (1 - fraction);

                //Insert the new value
                Complex2dx new_value(spot.value());
                new_value.set_amplitude(scaled_amplitude);
                new_data.set_value_at(index.h(), index.k(), index.l(), new_value, spot.weight());
            }
        }
    }
    
    set_fourier(new_data);
}


void ds::Volume2dx::write_bead_model_pdb(int no_of_beads, double density_threshold, double noise_level, std::string pdb_file)
{
    volume::utilities::BeadModelGenerator generator(no_of_beads, density_threshold, noise_level);
    generator.generate_bead_model_coordinates(*this, pdb_file);
}

ds::Volume2dx ds::Volume2dx::generate_bead_model(int no_of_beads, double density_threshold, double bead_model_resolution)
{
    ds::Volume2dx bead_model(header());
    volume::utilities::BeadModelGenerator generator(no_of_beads, density_threshold, 1.0, bead_model_resolution);
    
    ds::RealSpaceData bead_model_real_data = generator.generate_bead_model_volume(*this);
    bead_model.set_real(bead_model_real_data);
    
    return bead_model;
}


void ds::Volume2dx::invert_hand()
{
    FourierSpaceData data = get_fourier();
    FourierSpaceData new_data = data.inverted_data(3);
    set_fourier(new_data);
}

void ds::Volume2dx::centerize_density_along_z()
{
    std::cout <<"Centering the density..\n";
    FourierSpaceData data = get_fourier();
    FourierSpaceData new_data;
    for(FourierSpaceData::const_iterator itr=data.begin(); itr!=data.end(); ++itr)
    {
        MillerIndex index = (*itr).first;
        DiffractionSpot spot = (*itr).second;
        
        Complex2dx new_value = spot.value();
        new_value.set_phase(spot.phase()+M_PI*index.l());
        
        new_data.set_value_at(index.h(), index.k(), index.l(), new_value, spot.weight());
    }
    
    set_fourier(new_data);
    
}



void ds::Volume2dx::centerize_density_along_xyz()
{
    std::cout <<"Centering the density in x,y,z..\n";
    FourierSpaceData data = get_fourier();
    FourierSpaceData new_data;
    for(FourierSpaceData::const_iterator itr=data.begin(); itr!=data.end(); ++itr)
    {
        MillerIndex index = (*itr).first;
        DiffractionSpot spot = (*itr).second;
        
        Complex2dx new_value = spot.value();
        new_value.set_phase(spot.phase()+M_PI*index.h()+M_PI*index.k()+M_PI*index.l());
        
        new_data.set_value_at(index.h(), index.k(), index.l(), new_value, spot.weight());
    }
    
    set_fourier(new_data);
    
}


void ds::Volume2dx::apply_density_histogram(Volume2dx reference)
{
    apply_density_histogram(reference, 1.0);
}

void ds::Volume2dx::apply_density_histogram(Volume2dx reference, double fraction)
{
    std::cout << "Applying density histogram\n";
    //Check the fraction
    if(fraction < 0.0 || fraction > 1.0)
    {
        std::cerr << "ERROR! The density histogram fraction can only be between 0 and 1";
        return;
    }
    
    RealSpaceData ref_real = RealSpaceData(reference.get_real());
    this->prepare_real();
    
    //Check the size of reference
    if(ref_real.size() != _real.size())
    {
        std::cerr << "ERROR! The size of reference volume " << ref_real.size() 
                  << " does not match to this volume's size " << _real.size()
                  << std::endl;
        return;
    }
    
    
    double* sorted_ref_values = ref_real.density_sorted_values();
    int* sorted_ids = _real.density_sorted_ids();
    
    RealSpaceData new_data(nx(), ny(), nz());
    for(int id=0; id<new_data.size(); id++)
    {
        int sorted_id = sorted_ids[id];
        double old_density = _real.get_value_at(sorted_id);
        double new_density = sorted_ref_values[id]*fraction + old_density*(1-fraction);
        new_data.set_value_at(sorted_id, new_density);
    }
    
    //new_data->scale(new_data->min(), _real->max());
    this->set_real(new_data);
    
}

void ds::Volume2dx::apply_density_threshold(double limit, double fraction)
{
   RealSpaceData data = get_real();
   data.threshold(limit, fraction);
   set_real(data);
}

void ds::Volume2dx::apply_real_mask(const RealSpaceData& mask, double fraction)
{
    RealSpaceData new_data = get_real();
    new_data.apply_mask(mask, fraction);   
    set_real(new_data);   
}

void ds::Volume2dx::apply_density_slab(double height, double fraction, bool centered)
{
    std::cout << "Applying density slab along vertical direction to the volume.. \n";
    RealSpaceData new_data = get_real();
    new_data.vertical_slab(height, fraction, centered);
    this->set_real(new_data);
}

void ds::Volume2dx::band_pass(double low_resolution, double high_resolution)
{
    if(low_resolution <= 0) low_resolution = resolution_at(0, 0, 0);
    if(high_resolution <= 0 ) high_resolution = 0.0;
    
    std::cout << "Band passing the resolution in range (" << low_resolution << ", " << high_resolution << ")\n";
    if(low_resolution <= high_resolution)
    {
        std::cerr << "ERROR: Cannot apply band pass filter with low_resolution > high_resolution!!";
        return;
    }
    
    ds::FourierSpaceData current_data = get_fourier();
    FourierSpaceData new_data;
    for(FourierSpaceData::const_iterator itr=current_data.begin(); itr!=current_data.end(); ++itr)
    {
        //Get the data for current reflection
        MillerIndex index = (*itr).first;
        DiffractionSpot spot = (*itr).second;
        double resolution = resolution_at(index.h(), index.k(), index.l());
        if(resolution >= high_resolution && resolution <= low_resolution)
        {
            new_data.set_value_at(index.h(), index.k(), index.l(), spot.value(), spot.weight());
        }
        
    }
    
    set_fourier(new_data);
    
}

void ds::Volume2dx::replace_reflections(const FourierSpaceData& fourier_data)
{
    FourierSpaceData current = get_fourier();
    current.replace_reflections(fourier_data, 0.00001);
    set_fourier(current);
}

void ds::Volume2dx::change_amplitudes(const FourierSpaceData& fourier_data)
{
    FourierSpaceData current = get_fourier();
    current.change_amplitudes(fourier_data, 0.00001);
    set_fourier(current);
}

void ds::Volume2dx::low_pass(double high_resolution)
{
    prepare_fourier();
    std::cout << "Current maximum resolution = " << max_resolution() << " A\n";
    band_pass(-1, high_resolution);
    std::cout << "Current maximum resolution = " << max_resolution() << " A\n";
}

void ds::Volume2dx::low_pass_butterworth(double high_resolution)
{
    prepare_fourier();
    std::cout << "Current maximum resolution = " << max_resolution() << " A\n";
    
    //double low_resolution = std::max(std::max(nx(), ny()), nz());
    //double low_resolution = nx()*ny()*nz();
    //double omegaL = 1.0/low_resolution;
    double omegaH = 1.0/high_resolution;
    //double eps = 0.882;
    //double aa = 10.624;
    //double order = 2.0f*log10(eps/sqrt(aa*aa-1.0f))/log10(omegaL/omegaH);
    //omegaL = omegaL/pow(eps,2.0f/order); 
    double order = 16;
    
    std::cout << "Low passing using Butterworth filter (order = " << order << ") with expected maximum resolution: " << high_resolution << " A\n";
    
    ds::FourierSpaceData current_data = get_fourier();
    FourierSpaceData new_data;
    for(FourierSpaceData::const_iterator itr=current_data.begin(); itr!=current_data.end(); ++itr)
    {
        //Get the data for current reflection
        MillerIndex index = (*itr).first;
        DiffractionSpot spot = (*itr).second;
        double resolution = 1/resolution_at(index.h(), index.k(), index.l());
        double weight = sqrt(1.0/(1.0+pow(resolution/omegaH, order)));
        new_data.set_value_at(index.h(), index.k(), index.l(), spot.value()*weight, spot.weight());
        //std::cout << index.to_string() << "(" << 1/resolution << " A)" << " had weight of: " << weight << "\n";
        
    }
    
    set_fourier(new_data);
    
    std::cout << "Current maximum resolution = " << max_resolution() << " A\n";
    
}

void ds::Volume2dx::low_pass_gaussian(double high_resolution)
{
    prepare_fourier();
    std::cout << "Current maximum resolution = " << max_resolution() << " A\n";
    
    std::cout << "Gaussian low pass with expected highest resolution = " << high_resolution << " A\n";
    
    double omega_square = 4*high_resolution*high_resolution;
    
    ds::FourierSpaceData current_data = get_fourier();
    FourierSpaceData new_data;
    for(FourierSpaceData::const_iterator itr=current_data.begin(); itr!=current_data.end(); ++itr)
    {
        //Get the data for current reflection
        MillerIndex index = (*itr).first;
        DiffractionSpot spot = (*itr).second;
        double resolution = 1/resolution_at(index.h(), index.k(), index.l());
        double weight = exp(-1*resolution*resolution*omega_square);
        new_data.set_value_at(index.h(), index.k(), index.l(), spot.value()*weight, spot.weight());
        //std::cout << index.to_string() << "(" << 1/resolution << " A)" << " had weight of: " << weight << "\n";
        
    }
    
    set_fourier(new_data);
    
    std::cout << "Current maximum resolution = " << max_resolution() << " A\n";
    
}

ds::Volume2dx ds::Volume2dx::subsample(int factor)
{
    std::cout << "Sub-sampling volume by " << factor << " times.\n";
    
    int new_nx = factor*nx();
    int new_ny = factor*ny();
    int new_nz = factor*nz();
    
    VolumeHeader2dx head = header();
    head.set_mx(new_nx);
    head.set_my(new_ny);
    head.set_mz(new_nz);
    Volume2dx new_volume(head);
    
    RealSpaceData data = get_real();
    RealSpaceData new_data(new_nx, new_ny, new_nz);
    for(int ix=0; ix<new_nx; ix++)
    {
        for(int iy=0; iy<new_ny; iy++)
        {
            for(int iz=0; iz<new_nz; iz++)
            {   
                int data_x = (int) ix/factor;
                int data_y = (int) iy/factor;
                int data_z = (int) iz/factor;
                new_data.set_value_at(ix, iy, iz, data.get_value_at(data_x, data_y, data_z));
            }
        }
    }
    
    new_volume.set_real(new_data);
    
    return new_volume;
}

ds::Volume2dx ds::Volume2dx::extended_volume(int x_cells, int y_cells, int z_cells)
{
    std::cout << "Extending volume to " << x_cells+1 << " X " << y_cells+1 << " X " << z_cells+1 << " unit cells \n";
    
    int new_nx = (x_cells+1)*nx();
    int new_ny = (y_cells+1)*ny();
    int new_nz = (z_cells+1)*nz();
    
    VolumeHeader2dx new_header = header();
    new_header.reset_size(new_nx, new_ny, new_nz);
    Volume2dx new_volume(new_header);

    RealSpaceData data = get_real();
    RealSpaceData new_data(new_nx, new_ny, new_nz);
    for(int ix=0; ix<new_nx; ix++)
    {
        for(int iy=0; iy<new_ny; iy++)
        {
            for(int iz=0; iz<new_nz; iz++)
            {   
                int data_x = ix % nx();
                int data_y = iy % ny();
                int data_z = iz % nz();
                new_data.set_value_at(ix, iy, iz, data.get_value_at(data_x, data_y, data_z));
            }
        }
    }
    
    new_volume.set_real(new_data);
    
    return new_volume;
}

ds::Volume2dx ds::Volume2dx::zero_phases()
{
    std::cout << "Zeroing phases in volume \n";
    
    Volume2dx new_volume(header());
    
    FourierSpaceData data = get_fourier();
    FourierSpaceData new_data;
    for(FourierSpaceData::const_iterator itr=data.begin(); itr!=data.end(); ++itr)
    {
        //Get the data for current reflection
        MillerIndex index = (*itr).first;
        Complex2dx value = (*itr).second.value();
        value.set_phase(0);
        new_data.set_value_at(index.h(), index.k(), index.l(), value, (*itr).second.weight());
    }
    
    new_volume.set_fourier(new_data);
    
    return new_volume;
}

ds::Volume2dx ds::Volume2dx::spread_fourier_data()
{
    FourierSpaceData data = get_fourier();
    data.spread_data();
    
    Volume2dx new_volume(header());
    new_volume.set_fourier(data);
    return new_volume;
}

void ds::Volume2dx::extend_to_full_fourier() 
{
    FourierSpaceData data = get_fourier();
    FourierSpaceData new_data = data.get_full_fourier();
    set_fourier(new_data);
}

ds::Volume2dx ds::Volume2dx::apply_bfactor(double negative_temp_factor)
{
    std::cout << "Applying a negative B-factor of: " << negative_temp_factor << "\n";
    
    Volume2dx new_volume(header());
    
    FourierSpaceData data = get_fourier();
    FourierSpaceData new_data;
    for(FourierSpaceData::const_iterator itr=data.begin(); itr!=data.end(); ++itr)
    {
        //Get the data for current reflection
        MillerIndex index = (*itr).first;
        Complex2dx value = (*itr).second.value();
        double resolution = resolution_at(index.h(), index.k(), index.l());
        double weight = exp(-1*negative_temp_factor/(4*resolution*resolution));
        new_data.set_value_at(index.h(), index.k(), index.l(), value*weight, (*itr).second.weight());
    }
    
    new_volume.set_fourier(new_data);
    
    return new_volume;
}

double ds::Volume2dx::density_at(int x, int y, int z)
{
    return get_real().get_value_at(x,y,z);
}

int ds::Volume2dx::nx() const
{
    return _header.rows();   
}

int ds::Volume2dx::ny() const
{
    return _header.columns();
}

int ds::Volume2dx::nz() const
{
    return _header.sections();
}

int ds::Volume2dx::fx() const{
    return (int) (nx()/2+1);
}

int ds::Volume2dx::fy() const{
    return ny();
}

int ds::Volume2dx::fz() const{
    return nz();
}


int ds::Volume2dx::h_max() const{
    return fx()-1;
}

int ds::Volume2dx::k_max() const{
    return (int) fy()/2;
}

int ds::Volume2dx::l_max() const{
    return (int) fz()/2;
}

double ds::Volume2dx::xlen() const {
    return _header.xlen();
}

void ds::Volume2dx::set_xlen(double xlen)
{
    _header.set_xlen(xlen);
}

double ds::Volume2dx::ylen() const {
    return _header.ylen();
}

void ds::Volume2dx::set_ylen(double ylen)
{
    _header.set_ylen(ylen);
}

double ds::Volume2dx::zlen() const {
    return _header.zlen();
}

void ds::Volume2dx::set_zlen(double zlen)
{
    _header.set_zlen(zlen);
}

double ds::Volume2dx::gamma() const
{
    return _header.gamma();
}

void ds::Volume2dx::set_gamma_radians(double gamma)
{
    _header.set_gamma(gamma);
}

void ds::Volume2dx::set_gamma_degrees(double gamma)
{
    _header.set_gamma(volume::utilities::angle_utilities::DegreeToRadian(gamma));
}

std::string ds::Volume2dx::symmetry() const
{
    return _header.symmetry();
}

void ds::Volume2dx::set_symmetry(std::string symmetry)
{
    _header.set_symmetry(symmetry);
}