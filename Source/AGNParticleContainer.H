
#ifndef _AGNParticleContainer_H_
#define _AGNParticleContainer_H_

#include <AMReX_MultiFab.H>
#include <AMReX_MultiFabUtil.H>
#include "AMReX_Particles.H"

#include "NyxParticleContainer.H"

namespace {

struct GhostCommTag {

    GhostCommTag(int pid, int gid, int tid)
        : proc_id(pid), grid_id(gid), tile_id(tid)
    {}

    int proc_id;
    int grid_id;
    int tile_id;
};

bool operator<(const GhostCommTag& l, const GhostCommTag& r) {
    return (l.proc_id < r.proc_id ||
           (l.proc_id == r.proc_id && l.grid_id < r.grid_id) ||
           (l.proc_id == r.proc_id && l.grid_id == r.grid_id && l.tile_id < r.tile_id ));
}

}

class AGNParticleContainer
    : public NyxParticleContainer<1+BL_SPACEDIM+3>
{
public:

    using MyParIter = amrex::ParIter<1+BL_SPACEDIM+3>;
    using PairIndex = std::pair<int, int>;
    using GhostCommMap = std::map<GhostCommTag, amrex::Array<char> >;

    AGNParticleContainer (amrex::Amr* amr)
        : NyxParticleContainer<1+BL_SPACEDIM+3>(amr)
    { }

    virtual ~AGNParticleContainer () {}

    ///
    /// Invalidate particles in cells that are already occupied
    ///
    // void ComputeOverlap(int lev, amrex::MultiFab& density_to_subtract);
    void ComputeOverlap(int lev);

    ///
    /// This fills the ghost buffers for each tile with the proper data
    ///
    void fillGhosts(int lev);

    ///
    /// Each tile clears its ghosts, freeing the memory
    ///
    void clearGhosts(int lev);

    ///
    /// Write out all particles at a level
    ///
    void writeAllAtLevel(int lev);

private:

    ///
    /// Pack a particle's data into the proper neighbor buffer, or put it into
    /// the structure to be sent to the other processes
    ///
    void packGhostParticle(int lev,
                           const amrex::IntVect& neighbor_cell,
                           const amrex::BaseFab<int>& mask,
                           const ParticleType& p,
                           GhostCommMap& ghosts_to_comm);

    ///
    /// Perform the MPI communication neccesary to fill ghost buffers
    ///
    void fillGhostsMPI(GhostCommMap& ghosts_to_comm);

    const size_t pdata_size = BL_SPACEDIM*sizeof(RealType); // we communicate the position only for ghosts
    amrex::FabArray<amrex::BaseFab<int> > mask;
    std::map<PairIndex, amrex::Array<char> > ghosts;
};

#endif /* _AGNParticleContainer_H_ */
