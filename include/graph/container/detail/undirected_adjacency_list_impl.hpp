//
//	Author: J. Phillip Ratzloff
//

#ifndef UNDIRECTED_ADJ_LIST_IMPL_HPP
#define UNDIRECTED_ADJ_LIST_IMPL_HPP

#include <stdexcept>
#include <unordered_set>

namespace graph::container {

///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list
///

// ual_vertex_edge_list::const_iterator
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::reference
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::operator*() const {
  return *edge_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::pointer
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::operator->() const {
  return edge_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::operator++() {
  advance();
  return *this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::operator++(int) {
  const_iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::operator--() {
  retreat();
  return *this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::operator--(int) {
  const_iterator tmp(*this);
  --*this;
  return tmp;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::advance() {
  edge_type* start_edge = edge_; // Remember where we started for cycle detection

  vertex_edge_list_inward_link_type&  inward_link  = *edge_; // in.vertex_id_ == this->vertex_id_;
  vertex_edge_list_outward_link_type& outward_link = *edge_;
  if (inward_link.vertex_id_ == vertex_id_) {
    edge_ = inward_link.next_;
  } else {
    assert(outward_link.vertex_id_ == vertex_id_);
    edge_ = outward_link.next_;
  }

  // Self-loop detection: if we've cycled back to the starting edge, treat as end
  if (edge_ == start_edge) {
    edge_ = nullptr;
  }
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::retreat() {
  if (edge_) {
    vertex_edge_list_inward_link_type&  inward_link  = *edge_; // in.vertex_id_ == this->vertex_id_;
    vertex_edge_list_outward_link_type& outward_link = *edge_;
    if (inward_link.vertex_id_ == vertex_id_) {
      edge_ = inward_link.prev_;
    } else {
      assert(outward_link.vertex_id_ == vertex_id_);
      edge_ = outward_link.prev_;
    }
  } else {
    vertex_iterator u = graph_->try_find_vertex(vertex_id_);
    edge_             = &u->edge_back(*graph_);
  }
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
bool ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::operator==(
      const const_iterator& rhs) const noexcept {
  return edge_ == rhs.edge_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
bool ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::operator!=(
      const const_iterator& rhs) const noexcept {
  return !(*this == rhs);
}

// ual_vertex_edge_list::iterator
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::reference
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator::operator*() const {
  return *this->edge_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::pointer
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator::operator->() const {
  return this->edge_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator::operator++() {
  this->advance();
  return *this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator::operator++(int) {
  iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator::operator--() {
  this->retreat();
  return *this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator::operator--(int) {
  iterator tmp(*this);
  --*this;
  return tmp;
}

// ual_vertex_edge_list
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::size_type
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::size() const noexcept {
  return size_;
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
bool ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::empty() const noexcept {
  return size_ == 0;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::front() {
  return *head_;
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
const typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::front() const {
  return *head_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::back() {
  return *tail_;
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
const typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::back() const {
  return *tail_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
template <typename ListT>
void ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::link_front(
      edge_type& uv, ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>& uv_link) {
  using link_t = ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>;
  if (head_) {
    link_t& head_link = *static_cast<link_t*>(head_);
    uv_link.next_     = head_;
    head_link.prev_   = &uv;
    head_             = &uv;
  } else {
    head_ = tail_ = &uv;
  }
  ++size_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
template <typename ListT>
void ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::link_back(
      edge_type& uv, ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>& uv_link) {
  if (tail_) {
    vertex_edge_list_inward_link_type&  tail_inward_link  = static_cast<vertex_edge_list_inward_link_type&>(*tail_);
    vertex_edge_list_outward_link_type& tail_outward_link = static_cast<vertex_edge_list_outward_link_type&>(*tail_);
    if (tail_inward_link.vertex_id_ == uv_link.vertex_id_) {
      uv_link.prev_          = tail_;
      tail_inward_link.next_ = &uv;
      tail_                  = &uv;
    } else {
      uv_link.prev_           = tail_;
      tail_outward_link.next_ = &uv;
      tail_                   = &uv;
    }
  } else {
    assert(!head_ && !tail_ && size_ == 0);
    head_ = tail_ = &uv;
  }
  ++size_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
template <typename ListT>
void ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::unlink(
      edge_type& uv, ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>& uv_link) {

  if (uv_link.prev_) {
    vertex_edge_list_inward_link_type& prev_inward_link =
          static_cast<vertex_edge_list_inward_link_type&>(*uv_link.prev_);
    vertex_edge_list_outward_link_type& prev_outward_link =
          static_cast<vertex_edge_list_outward_link_type&>(*uv_link.prev_);
    if (prev_inward_link.vertex_id_ == uv_link.vertex_id_) {
      prev_inward_link.next_ = uv_link.next_;
    } else {
      assert(prev_outward_link.vertex_id_ == uv_link.vertex_id_);
      prev_outward_link.next_ = uv_link.next_;
    }
  }
  if (tail_ == &uv) {
    tail_ = uv_link.prev_;
  }

  if (uv_link.next_) {
    vertex_edge_list_inward_link_type& next_inward_link =
          static_cast<vertex_edge_list_inward_link_type&>(*uv_link.next_);
    vertex_edge_list_outward_link_type& next_outward_link =
          static_cast<vertex_edge_list_outward_link_type&>(*uv_link.next_);
    if (next_inward_link.vertex_id_ == uv_link.vertex_id_) {
      next_inward_link.prev_ = uv_link.prev_;
    } else {
      assert(next_outward_link.vertex_id_ == uv_link.vertex_id_);
      next_outward_link.prev_ = uv_link.prev_;
    }
  }
  if (head_ == &uv) {
    head_ = uv_link.next_;
  }

  uv_link.prev_ = uv_link.next_ = nullptr;
  --size_;

  if (size_ == 0) {
    assert(head_ == nullptr && tail_ == nullptr);
  }
  // Note: For self-loops, size_ may be > 0 but the edge appears in the list twice
  // (once as inward, once as outward), so assertions about head/tail equality
  // don't hold during partial unlink. The invariant is restored after both unlinks.
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::begin(graph_type& g, vertex_id_type uid) noexcept {
  return iterator(g, uid, head_);
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::begin(const graph_type& g,
                                                                vertex_id_type    uid) const noexcept {
  return const_iterator(g, uid, head_);
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::cbegin(const graph_type& g,
                                                                 vertex_id_type    uid) const noexcept {
  return const_iterator(g, uid, head_);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::end(graph_type& g, vertex_id_type uid) noexcept {
  return iterator(g, uid, nullptr);
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::end(const graph_type& g, vertex_id_type uid) const noexcept {
  return const_iterator(g, uid, nullptr);
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::cend(const graph_type& g, vertex_id_type uid) const noexcept {
  return const_iterator(g, uid, nullptr);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::edge_range
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::edges(graph_type& g, vertex_id_type uid) noexcept {
  return {iterator(g, uid, head_), iterator(g, uid, nullptr), size_};
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_range
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::edges(const graph_type& g,
                                                                vertex_id_type    uid) const noexcept {
  return {const_iterator(g, uid, head_), const_iterator(g, uid, nullptr), size_};
}

///-------------------------------------------------------------------------------------
/// ual_edge
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::ual_edge(graph_type& g, vertex_id_type uid, vertex_id_type vid) noexcept
      : base_value_type(), vertex_edge_list_inward_link_type(uid), vertex_edge_list_outward_link_type(vid) {
  link_back(g.vertices()[uid], g.vertices()[vid]);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&            g,
                                                       vertex_id_type         uid,
                                                       vertex_id_type         vid,
                                                       const edge_value_type& val) noexcept
      : base_value_type(val), vertex_edge_list_inward_link_type(uid), vertex_edge_list_outward_link_type(vid) {
  link_back(g.vertices()[uid], g.vertices()[vid]);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&       g,
                                                       vertex_id_type    uid,
                                                       vertex_id_type    vid,
                                                       edge_value_type&& val) noexcept
      : base_value_type(move(val)), vertex_edge_list_inward_link_type(uid), vertex_edge_list_outward_link_type(vid) {
  link_back(g.vertices()[uid], g.vertices()[vid]);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::ual_edge(graph_type& g, vertex_iterator ui, vertex_iterator vi) noexcept
      : base_value_type()
      , vertex_edge_list_inward_link_type(static_cast<vertex_id_type>(ui - g.vertices().begin()))
      , vertex_edge_list_outward_link_type(static_cast<vertex_id_type>(vi - g.vertices().begin())) {
  link_back(*ui, *vi);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&            g,
                                                       vertex_iterator        ui,
                                                       vertex_iterator        vi,
                                                       const edge_value_type& val) noexcept
      : base_value_type(val)
      , vertex_edge_list_inward_link_type(static_cast<vertex_id_type>(ui - g.vertices().begin()))
      , vertex_edge_list_outward_link_type(static_cast<vertex_id_type>(vi - g.vertices().begin())) {
  link_back(*ui, *vi);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&       g,
                                                       vertex_iterator   ui,
                                                       vertex_iterator   vi,
                                                       edge_value_type&& val) noexcept
      : base_value_type(move(val))
      , vertex_edge_list_inward_link_type(static_cast<vertex_id_type>(ui - g.vertices().begin()))
      , vertex_edge_list_outward_link_type(static_cast<vertex_id_type>(vi - g.vertices().begin())) {
  link_back(*ui, *vi);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::~ual_edge() noexcept {
  [[maybe_unused]] vertex_edge_list_outward_link_type& outward_link =
      *static_cast<vertex_edge_list_outward_link_type*>(this);
  assert(outward_link.prev() == nullptr && outward_link.next() == nullptr); // has edge been unlinked?

  [[maybe_unused]] vertex_edge_list_inward_link_type& inward_link =
      *static_cast<vertex_edge_list_inward_link_type*>(this);
  assert(inward_link.prev() == nullptr && inward_link.next() == nullptr); // has edge been unlinked?
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void ual_edge<EV, VV, GV, VId, VContainer, Alloc>::link_front(vertex_type& u, vertex_type& v) noexcept {
  u.edges_.link_front(*this, *static_cast<vertex_edge_list_inward_link_type*>(this));
  v.edges_.link_front(*this, *static_cast<vertex_edge_list_outward_link_type*>(this));
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void ual_edge<EV, VV, GV, VId, VContainer, Alloc>::link_back(vertex_type& u, vertex_type& v) noexcept {
  u.edges_.link_back(*this, *static_cast<vertex_edge_list_inward_link_type*>(this));
  v.edges_.link_back(*this, *static_cast<vertex_edge_list_outward_link_type*>(this));
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void ual_edge<EV, VV, GV, VId, VContainer, Alloc>::unlink(vertex_type& u, vertex_type& v) noexcept {
  u.edges_.unlink(*this, *static_cast<vertex_edge_list_inward_link_type*>(this));
  v.edges_.unlink(*this, *static_cast<vertex_edge_list_outward_link_type*>(this));
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::source(graph_type& g) noexcept {
  return g.vertices().begin() + list_owner_id();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::source(const graph_type& g) const noexcept {
  return static_cast<vertex_edge_list_inward_link_type const*>(this)->vertex(g);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::vertex_id_type
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::list_owner_id() const noexcept {
  return static_cast<vertex_edge_list_inward_link_type const*>(this)->vertex_id();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::target(graph_type& g) noexcept {
  return g.vertices().begin() + list_target_id();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::target(const graph_type& g) const noexcept {
  return static_cast<vertex_edge_list_outward_link_type const*>(this)->vertex(g);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::vertex_id_type
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::list_target_id() const noexcept {
  return static_cast<vertex_edge_list_outward_link_type const*>(this)->vertex_id();
}


template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::other_vertex(graph_type& g, const_vertex_iterator other) noexcept {
  return other != source(g) ? source(g) : target(g);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::other_vertex(const graph_type&     g,
                                                           const_vertex_iterator other) const noexcept {
  return other != source(g) ? source(g) : target(g);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::other_vertex(graph_type& g, vertex_id_type other_id) noexcept {
  return other_id != list_owner_id() ? source(g) : target(g);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::other_vertex(const graph_type& g,
                                                           vertex_id_type    other_id) const noexcept {
  return other_id != list_owner_id() ? source(g) : target(g);
}


template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::vertex_id_type
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::other_vertex_id(const graph_type&     g,
                                                              const_vertex_iterator other) const noexcept {
  return other != source(g) ? list_owner_id() : list_target_id();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::vertex_id_type
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::other_vertex_id([[maybe_unused]] const graph_type& g,
                                                              vertex_id_type other_id) const noexcept {
  return other_id != list_owner_id() ? list_owner_id() : list_target_id();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_edge<EV, VV, GV, VId, VContainer, Alloc>::edge_id_type
ual_edge<EV, VV, GV, VId, VContainer, Alloc>::edge_id(const graph_type& g) const noexcept {
  return unordered_pair(list_owner_id(), list_target_id());
}


///-------------------------------------------------------------------------------------
/// ual_vertex
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::ual_vertex([[maybe_unused]] vertex_set&  vertex_store,
                                                           [[maybe_unused]] vertex_index index) {}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::ual_vertex([[maybe_unused]] vertex_set&  vertex_store,
                                                           [[maybe_unused]] vertex_index index,
                                                           const vertex_value_type&      val)
      : base_value_type(val) {}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::ual_vertex([[maybe_unused]] vertex_set&  vertex_store,
                                                           [[maybe_unused]] vertex_index index,
                                                           vertex_value_type&&           val) noexcept
      : base_value_type(move(val)) {}


template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edges_begin(graph_type& g, vertex_id_type uid) noexcept {
  return edges_.begin(g, uid);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edges_begin(const graph_type& g, vertex_id_type uid) const noexcept {
  return edges_.begin(g, uid);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edges_cbegin(const graph_type& g, vertex_id_type uid) const noexcept {
  return edges_.cbegin(g, uid);
}

// Removed: e_begin implementation - legacy method using const_cast, replaced by edges_begin


template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edges_end(graph_type& g, vertex_id_type uid) noexcept {
  return edges_.end(g, uid);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edges_end(const graph_type& g, vertex_id_type uid) const noexcept {
  return edges_.end(g, uid);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edges_cend(const graph_type& g, vertex_id_type uid) const noexcept {
  return edges_.cend(g, uid);
}

// Removed: e_end implementation - legacy method using const_cast, replaced by edges_end

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_range
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edges(graph_type& g, vertex_id_type uid) {
  return edges_.edges(g, uid);
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_edge_range
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edges(const graph_type& g, vertex_id_type uid) const {
  return edges_.edges(g, uid);
}


#if 0
template <typename EV, typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_id_type
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_id(const graph_type& g) const noexcept {
  return static_cast<vertex_id_type>(this - g.vertices().data());
}
#endif


template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edge_front(graph_type& g) noexcept {
  return edges_.front(g, *this);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
const typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edge_front(const graph_type& g) const noexcept {
  return edges_.front(g, *this);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edge_back(graph_type& g) noexcept {
  return edges_.back();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
const typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::edge_back(const graph_type& g) const noexcept {
  return edges_.back();
}


template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_size_type
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::num_edges() const {
  return edges_.size();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::erase_edge(graph_type& g, edge_type* uv) {
  vertex_type& u = g.vertices()[uv->list_owner_id()];
  vertex_type& v = g.vertices()[uv->list_target_id()];
  uv->unlink(u, v);

  uv->~edge_type();
  g.edge_alloc_.deallocate(uv, 1);
  --g.edges_size_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::clear_edges(graph_type& g) {
  while (!edges_.empty()) {
    erase_edge(g, &edges_.front());
  }
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::erase_edge(graph_type& g, vertex_edge_iterator uvi) {
  edge_type* uv = &*uvi;
  ++uvi;
  erase_edge(g, uv);
  return uvi;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::erase_edge(graph_type&          g,
                                                           vertex_edge_iterator first,
                                                           vertex_edge_iterator last) {
  while (first != last)
    first = erase_edge(g, first);
  return first;
}

#if 0
template <typename EV, typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::add_edge(graph_type& g, vertex_type& v) {
  edge_type* uv = g.edge_alloc_.allocate(1);
  new (uv) edge_type(g, *this, v);
  ++g.edges_size_;
  return vertex_edge_iterator(g, *this, uv);
}

template <typename EV, typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::add_edge(graph_type& g, vertex_type& v, edge_value_type&& val) {
  edge_type* uv = g.edge_alloc_.allocate(1);
  new (uv) edge_type(g, *this, v, move(val));
  ++g.edges_size_;
  return vertex_edge_iterator(g, *this, uv);
}

template <typename EV, typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::add_edge(graph_type& g, vertex_type& v, const edge_value_type& val) {
  edge_type* uv = g.edge_alloc_.allocate(1);
  new (uv) edge_type(g, *this, v, val);
  ++g.edges_size_;
  return vertex_edge_iterator(g, *this, uv);
}
#endif

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::neighbor_size_type
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::neighbors_size(const graph_type& g) const {
  return size(edges(g));
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::neighbor_range
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::neighbors(graph_type& g, vertex_id_type uid) {
  return {neighbor_iterator(edges_.begin(g, uid)), neighbor_iterator(edges_.end(g, uid)), edges_.size()};
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::const_neighbor_range
ual_vertex<EV, VV, GV, VId, VContainer, Alloc>::neighbors(const graph_type& g, vertex_id_type uid) const {
  return {const_neighbor_iterator(edges_.begin(g, uid)), const_neighbor_iterator(edges_.end(g, uid)), edges_.size()};
}


///-------------------------------------------------------------------------------------
/// base_undirected_adjacency_list
///

// Copy constructor - copies vertices and edges
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::base_undirected_adjacency_list(
      const base_undirected_adjacency_list& other) {
  // Copy base class members
  edge_alloc_ = other.edge_alloc_;

  // Reserve space and copy vertices (with empty edge lists)
  vertices_.reserve(other.vertices_.size());

  for (const auto& v : other.vertices_) {
    if constexpr (std::is_void_v<VV>) {
      vertices_.push_back(vertex_type(vertices_, static_cast<vertex_id_type>(vertices_.size())));
    } else {
      // Access vertex value member directly
      vertices_.push_back(vertex_type(vertices_, static_cast<vertex_id_type>(vertices_.size()), v.value()));
    }
  }

  // Downcast to graph_type to access methods
  auto& g = static_cast<graph_type&>(*this);

  // Copy edges - iterate through each vertex and copy edges where source_id <= target_id to avoid duplicates
  for (vertex_id_type uid = 0; uid < static_cast<vertex_id_type>(other.vertices_.size()); ++uid) {
    const auto& src_vtx = other.vertices_[uid];
    for (auto uv = src_vtx.edges_begin(static_cast<const graph_type&>(other), uid);
         uv != src_vtx.edges_end(static_cast<const graph_type&>(other), uid); ++uv) {
      vertex_id_type src_id = uv->list_owner_id();
      vertex_id_type tgt_id = uv->list_target_id();
      // Only copy each edge once: when uid matches source and source <= target
      if (uid == src_id && src_id <= tgt_id) {
        if constexpr (std::is_void_v<EV>) {
          g.add_edge(src_id, tgt_id);
        } else {
          g.add_edge(src_id, tgt_id, uv->value());
        }
      }
    }
  }
}

// Range constructor
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
template <typename ERng, typename VRng, typename EProj, typename VProj>
requires ranges::forward_range<ERng> && ranges::input_range<VRng> &&
               std::regular_invocable<EProj, ranges::range_reference_t<ERng>> &&
               std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::base_undirected_adjacency_list(
      const ERng& erng, const VRng& vrng, const EProj& eproj, const VProj& vproj, const allocator_type& alloc)
      : vertices_(alloc), edge_alloc_(alloc) {
  // Handle empty case - no vertices or edges to create
  if (vrng.empty() && ranges::empty(erng)) {
    return;
  }

  // Evaluate max vertex id needed
  vertex_id_type max_vtx_id = vrng.empty() ? vertex_id_type(0) : static_cast<vertex_id_type>(vrng.size() - 1);
  for (auto& e : erng) {
    auto&& edge_data = eproj(e); // copyable_edge_t<VId, EV>
    max_vtx_id       = max(max_vtx_id, max(edge_data.source_id, edge_data.target_id));
  }

  // add vertices
  vertices_.reserve(max_vtx_id + 1);
  if constexpr (!std::is_void_v<VV>) {
    for (auto& vtx : vrng) {
      auto&& vtx_info = vproj(vtx); // copyable_vertex_t<VId, VV>
      vertices_.push_back(vertex_type(vertices_, static_cast<vertex_id_type>(vertices_.size()), vtx_info.value));
    }
  }
  vertices_.resize(max_vtx_id + 1); // assure expected vertices exist

  // Downcast to graph_type to access add_edge
  auto& g = static_cast<graph_type&>(*this);

  // add edges
  if (!ranges::empty(erng)) {
    auto&&         first_edge_data = eproj(*ranges::begin(erng)); // first edge
    vertex_id_type tid             = first_edge_data.source_id;   // last in-vertex id
    for (auto& edge_data : erng) {
      auto&& ed = eproj(edge_data); // copyable_edge_t<VId, EV>
      if (ed.source_id < tid)
        g.throw_unordered_edges();

      if constexpr (std::is_void_v<EV>) {
        g.add_edge(ed.source_id, ed.target_id);
      } else {
        g.add_edge(ed.source_id, ed.target_id, ed.value);
      }
      tid = ed.source_id;
    }
  }
}

// Initializer list constructor with edge values
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::base_undirected_adjacency_list(
      const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist,
      const allocator_type&                                                           alloc)
      : vertices_(alloc), edge_alloc_(alloc) {
  // Evaluate max vertex id needed
  vertex_id_type max_vtx_id = vertex_id_type();
  for (auto& edge_data : ilist) {
    const auto& [uid, vid, uv_val] = edge_data;
    max_vtx_id                     = max(max_vtx_id, max(uid, vid));
  }
  vertices_.resize(max_vtx_id + 1); // assure expected vertices exist

  // Downcast to graph_type to access add_edge
  auto& g = static_cast<graph_type&>(*this);

  // add edges - no ordering requirement, just insert them
  for (auto& edge_data : ilist) {
    const auto& [uid, vid, uv_val] = edge_data;
    g.add_edge(uid, vid, uv_val);
  }
}

// Initializer list constructor without edge values
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::base_undirected_adjacency_list(
      const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist, const allocator_type& alloc)
      : vertices_(alloc), edge_alloc_(alloc) {
  // Evaluate max vertex id needed
  vertex_id_type max_vtx_id = vertex_id_type();
  for (auto& edge_data : ilist) {
    const auto& [uid, vid] = edge_data;
    max_vtx_id             = max(max_vtx_id, max(uid, vid));
  }
  vertices_.resize(max_vtx_id + 1); // assure expected vertices exist

  // Downcast to graph_type to access add_edge
  auto& g = static_cast<graph_type&>(*this);

  // add edges - no ordering requirement, just insert them
  for (auto& edge_data : ilist) {
    const auto& [uid, vid] = edge_data;
    g.add_edge(uid, vid);
  }
}


// Destructor
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::~base_undirected_adjacency_list() {
  // Downcast to graph_type to access clear() method
  auto& g = static_cast<graph_type&>(*this);
  g.clear(); // assure edges are deleted using edge_alloc_
}

// Copy assignment operator
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>&
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::operator=(
      const base_undirected_adjacency_list& other) {
  if (this != &other) {
    // Use copy-and-swap idiom
    // Note: This requires derived class to have proper copy constructor
    base_undirected_adjacency_list tmp(other);

    // Swap base members
    vertices_.swap(tmp.vertices_);
    std::swap(edges_size_, tmp.edges_size_);
    std::swap(vertex_alloc_, tmp.vertex_alloc_);
    std::swap(edge_alloc_, tmp.edge_alloc_);
  }
  return *this;
}

// Helper method for validation
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::throw_unordered_edges() const {
  assert(false); // container must be sorted by edge_id.first
  throw std::invalid_argument("edges not ordered");
}

//-------------------------------------------------------------------------------------
// Accessor methods
//-------------------------------------------------------------------------------------

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_allocator_type
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_allocator() const noexcept {
  return this->edge_alloc_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_set&
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertices() {
  return this->vertices_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr const typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_set&
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertices() const {
  return this->vertices_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::begin() {
  return this->vertices_.begin();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::begin() const {
  return this->vertices_.begin();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::cbegin() const {
  return this->vertices_.cbegin();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::end() {
  return this->vertices_.end();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::end() const {
  return this->vertices_.end();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::cend() const {
  return this->vertices_.cend();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::try_find_vertex(vertex_id_type id) {
  if (id < this->vertices_.size())
    return this->vertices_.begin() + id;
  else
    return this->vertices_.end();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::try_find_vertex(vertex_id_type id) const {
  if (id < this->vertices_.size())
    return this->vertices_.begin() + id;
  else
    return this->vertices_.end();
}


//-------------------------------------------------------------------------------------
// Edge creation methods
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// Edge removal methods
//-------------------------------------------------------------------------------------

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::remove_edge(edge_iterator pos) {
  edge_type* uv = &*pos;
  ++pos;
  vertex_type& u = this->vertices_[uv->list_owner_id()];
  vertex_type& v = this->vertices_[uv->list_target_id()];
  uv->unlink(u, v); // unlink from both endpoints' edge lists
  uv->~edge_type();
  this->edge_alloc_.deallocate(uv, 1);
  --this->edges_size_;
  return pos;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_size_type
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::remove_edge(vertex_id_type uid,
                                                                                vertex_id_type vid) {
  auto& derived = static_cast<graph_type&>(*this);
  if (uid >= this->vertices_.size() || vid >= this->vertices_.size())
    throw std::out_of_range("remove_edge: vertex id out of range");

  vertex_type&         u     = this->vertices_[uid];
  vertex_size_type     count = 0;
  vertex_edge_iterator it    = u.edges_begin(derived, uid);
  vertex_edge_iterator last  = u.edges_end(derived, uid);
  while (it != last) {
    edge_type&     uv    = *it;
    vertex_id_type other = (uv.list_owner_id() == uid) ? uv.list_target_id() : uv.list_owner_id();
    if (other == vid) {
      it = u.erase_edge(derived, it); // unlinks from both endpoints, returns next in u's list
      ++count;
    } else {
      ++it;
    }
  }
  return count;
}

//-------------------------------------------------------------------------------------
// Vertex removal methods
//-------------------------------------------------------------------------------------

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::remove_vertex(vertex_id_type uid) {
  auto& derived = static_cast<graph_type&>(*this);
  if (uid >= this->vertices_.size())
    throw std::out_of_range("remove_vertex: vertex id out of range");

  // 1. Remove all edges incident to the vertex being erased.
  this->vertices_[uid].clear_edges(derived);

  // 2. Collect the remaining (unique) physical edges. Each undirected edge appears twice
  //    when iterating, so deduplicate by address. Ids are still consistent at this point.
  std::unordered_set<edge_type*> remaining;
  for (edge_type& e : derived.edges())
    remaining.insert(&e);

  // 3. Erase the vertex; vertices with a higher id shift down by one position.
  this->vertices_.erase(this->vertices_.begin() + static_cast<vertex_difference_type>(uid));

  // 4. Renumber the stored endpoint ids of every remaining edge.
  for (edge_type* e : remaining)
    e->renumber_after_vertex_erase(uid);
}

//-------------------------------------------------------------------------------------
// Graph modification methods
//-------------------------------------------------------------------------------------

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::clear() {
  // make sure edges are deallocated from this->edge_alloc_
  // Use downcast to call derived class clear_edges method
  auto& derived = static_cast<graph_type&>(*this);
  for (vertex_type& u : this->vertices_)
    u.clear_edges(derived);
  this->vertices_.clear(); // now we can clear the vertices
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::swap(base_undirected_adjacency_list& other) {
  using std::swap;
  this->vertices_.swap(other.vertices_);
  swap(this->edges_size_, other.edges_size_);
  swap(this->vertex_alloc_, other.vertex_alloc_);
  swap(this->edge_alloc_, other.edge_alloc_);
  // Note: Does NOT swap graph_value_ - that's handled by derived class
}

//-------------------------------------------------------------------------------------
// Utility methods
//-------------------------------------------------------------------------------------

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::reserve_vertices(vertex_size_type n) {
  this->vertices_.reserve(n);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::resize_vertices(vertex_size_type n) {
  this->vertices_.resize(n);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::resize_vertices(vertex_size_type         n,
                                                                                         const vertex_value_type& val) {
  this->vertices_.resize(n, val);
}


///-------------------------------------------------------------------------------------
/// undirected_adjacency_list
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const allocator_type& alloc)
      : base_type(alloc) {}


template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const graph_value_type& val,
                                                                                         const allocator_type&   alloc)
      : base_type(alloc), graph_value_(val) {}


template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(graph_value_type&&    val,
                                                                                         const allocator_type& alloc)
      : base_type(alloc), graph_value_(std::forward<graph_value_type>(val)) {}


// clang-format off
template <typename EV, typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
template <typename ERng, typename VRng, typename EProj, typename VProj, typename GV_>
  requires ranges::forward_range<ERng> 
        && ranges::input_range<VRng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
        && (!std::is_void_v<GV_>)
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const ERng&  erng,
                                                                              const VRng&  vrng,
                                                                              const EProj& eproj,
                                                                              const VProj& vproj,
                                                                              const GV_&   gv,
                                                                              const Alloc& alloc)
      : base_type(erng, vrng, eproj, vproj, alloc)
      , graph_value_(gv)
// clang-format on
{}

// clang-format off
template <typename EV, typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
template <typename ERng, typename EProj, typename GV_>
  requires ranges::forward_range<ERng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && (!std::is_void_v<GV_>)
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const ERng&  erng, 
                                                                              const EProj& eproj, 
                                                                              const GV_&   gv, 
                                                                              const Alloc& alloc)
      : undirected_adjacency_list(erng, vector<int>(), eproj, [](auto) 
{ return copyable_vertex_t<VId, VV>{.id = VId(), .value = VV{}}; }, gv, alloc)
// clang-format on
{}

// Overload for GV=void: edge+vertex range constructor without graph value
// clang-format off
template <typename EV, typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
template <typename ERng, typename VRng, typename EProj, typename VProj>
  requires ranges::forward_range<ERng>
        && ranges::input_range<VRng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
        && std::is_void_v<GV>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const ERng&  erng,
                                                                              const VRng&  vrng,
                                                                              const EProj& eproj,
                                                                              const VProj& vproj,
                                                                              const Alloc& alloc)
      : base_type(alloc)
// clang-format on
{
  // Handle empty case - no vertices or edges to create
  if (vrng.empty() && ranges::empty(erng)) {
    return;
  }

  // Evaluate max vertex id needed
  vertex_id_type max_vtx_id = vrng.empty() ? vertex_id_type(0) : static_cast<vertex_id_type>(vrng.size() - 1);
  for (auto& e : erng) {
    auto&& edge_data = eproj(e); // copyable_edge_t<VId, EV>
    max_vtx_id       = max(max_vtx_id, max(edge_data.source_id, edge_data.target_id));
  }

  // add vertices
  this->vertices_.reserve(max_vtx_id + 1);
  if constexpr (!std::is_void_v<VV>) {
    for (auto& vtx : vrng) {
      auto&& [id, value] = vproj(vtx); // copyable_vertex_t<VId, VV>
      add_vertex(value);
    }
  }
  this->vertices_.resize(max_vtx_id + 1); // assure expected vertices exist

  // add edges
  if (!ranges::empty(erng)) {
    auto&&         first_edge_data = eproj(*ranges::begin(erng)); // first edge
    vertex_id_type tid             = first_edge_data.source_id;   // last in-vertex id
    for (auto& edge_data : erng) {
      auto&& ed = eproj(edge_data); // copyable_edge_t<VId, EV>
      if (ed.source_id < tid)
        this->throw_unordered_edges();

      vertex_edge_iterator uv;
      if constexpr (std::is_void_v<EV>) {
        uv = add_edge(ed.source_id, ed.target_id);
      } else {
        uv = add_edge(ed.source_id, ed.target_id, ed.value);
      }
      tid = ed.source_id;
    }
  }
}

// Overload for GV=void: edge-only range constructor without graph value
// clang-format off
template <typename EV, typename VV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
template <typename ERng, typename EProj>
  requires ranges::forward_range<ERng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && std::is_void_v<GV>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const ERng&  erng, 
                                                                              const EProj& eproj, 
                                                                              const Alloc& alloc)
      : undirected_adjacency_list(erng, vector<int>(), eproj, [](auto) 
{ return copyable_vertex_t<VId, VV>{VId()}; }, alloc)
// clang-format on
{}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(
      const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist, const Alloc& alloc)
      : base_type(ilist, alloc) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(
      const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist, const Alloc& alloc)
      : base_type(ilist, alloc) {}

// Copy constructor - deep copies all vertices and edges
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(
      const undirected_adjacency_list& other)
      : base_type(other), graph_value_(other.graph_value_) {}

// Copy assignment operator - uses copy-and-swap idiom
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>&
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::operator=(const undirected_adjacency_list& other) {
  if (this != &other) {
    undirected_adjacency_list tmp(other);
    swap(tmp);
  }
  return *this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::~undirected_adjacency_list() = default;

//-------------------------------------------------------------------------------------
// Vertex/edge modification methods (non-accessor)
//-------------------------------------------------------------------------------------

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::swap(undirected_adjacency_list& rhs) {
  using std::swap;
  swap(graph_value_, rhs.graph_value_);
  base_type::swap(rhs); // Call base class swap for vertices, edges_size, allocators
}

///-------------------------------------------------------------------------------------
/// ual_const_neighbor_iterator
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::ual_const_neighbor_iterator(
      vertex_edge_iterator const& uv)
      : uv_(uv) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::graph_type&
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::graph() noexcept {
  return uv_.graph();
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr const typename ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::graph_type&
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::graph() const noexcept {
  return uv_.graph();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::other_vertex() const {
  return uv_->other_vertex(uv_.graph(), uv_.source_id());
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::vertex_id_type
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::other_vertex_id() const {
  return uv_->other_vertex_id(uv_.graph(), uv_.source_id());
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::reference
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator*() const noexcept {
  return *uv_->other_vertex(uv_.graph(), uv_.source_id());
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::pointer
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator->() const noexcept {
  return &**this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>&
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator++() noexcept {
  ++uv_;
  return *this;
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator++(int) noexcept {
  ual_const_neighbor_iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>&
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator--() noexcept {
  --uv_;
  return *this;
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>
ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator--(int) noexcept {
  ual_const_neighbor_iterator tmp(*this);
  --*this;
  return tmp;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr bool ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator==(
      const ual_const_neighbor_iterator& rhs) const noexcept {
  return uv_ == rhs.uv_;
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr bool ual_const_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator!=(
      const ual_const_neighbor_iterator& rhs) const noexcept {
  return !operator==(rhs);
}


///-------------------------------------------------------------------------------------
/// ual_neighbor_iterator
///
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::ual_neighbor_iterator(
      vertex_edge_iterator const& uv)
      : base_t(uv) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::other_vertex() {
  return uv_->other_vertex(uv_.graph(), uv_.source_id());
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::const_reference
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator*() const {
  return *uv_->other_vertex(uv_.graph(), uv_.source_id());
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::const_pointer
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator->() const {
  return &**this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>&
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator++() {
  ++uv_;
  return *this;
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator++(int) {
  ual_neighbor_iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>&
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator--() noexcept {
  --uv_;
  return *this;
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator--(int) noexcept {
  ual_neighbor_iterator tmp(*this);
  --*this;
  return tmp;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr bool
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator==(const ual_neighbor_iterator& rhs) const noexcept {
  return base_t::operator==(rhs);
}
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr bool
ual_neighbor_iterator<EV, VV, GV, VId, VContainer, Alloc>::operator!=(const ual_neighbor_iterator& rhs) const noexcept {
  return base_t::operator!=(rhs);
}

///-------------------------------------------------------------------------------------
/// Implementations for undirected_adjacency_list<EV, VV, void, ...> specialization
/// (when GV=void, no graph_value_ member)
///

// Default allocator constructor
template <typename EV, typename VV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>::undirected_adjacency_list(const allocator_type& alloc)
      : base_type(alloc) {}

// Copy constructor
template <typename EV, typename VV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>::undirected_adjacency_list(
      const undirected_adjacency_list& other)
      : base_type(other) {}

// Initializer list constructors
template <typename EV, typename VV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>::undirected_adjacency_list(
      const initializer_list<tuple<vertex_id_type, vertex_id_type, edge_value_type>>& ilist, const Alloc& alloc)
      : base_type(ilist, alloc) {}

template <typename EV, typename VV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>::undirected_adjacency_list(
      const initializer_list<tuple<vertex_id_type, vertex_id_type>>& ilist, const Alloc& alloc)
      : base_type(ilist, alloc) {}

// Destructor
template <typename EV, typename VV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>::~undirected_adjacency_list() = default;

//-------------------------------------------------------------------------------------
// Vertex/edge modification methods (non-accessor) - GV=void specialization
//-------------------------------------------------------------------------------------

template <typename EV, typename VV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
void undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>::swap(undirected_adjacency_list& rhs) {
  base_type::swap(rhs); // Call base class swap (no graph_value_ to swap)
}

template <typename EV, typename VV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>&
undirected_adjacency_list<EV, VV, void, VId, VContainer, Alloc>::operator=(const undirected_adjacency_list& other) {
  if (this != &other) {
    undirected_adjacency_list tmp(other);
    swap(tmp);
  }
  return *this;
}

///-------------------------------------------------------------------------------------
/// ual_edge_value
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_edge_value<EV, VV, GV, VId, VContainer, Alloc>::value_type&
ual_edge_value<EV, VV, GV, VId, VContainer, Alloc>::value() noexcept {
  return value_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr const typename ual_edge_value<EV, VV, GV, VId, VContainer, Alloc>::value_type&
ual_edge_value<EV, VV, GV, VId, VContainer, Alloc>::value() const noexcept {
  return value_;
}

///-------------------------------------------------------------------------------------
/// ual_vertex_value
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename ual_vertex_value<EV, VV, GV, VId, VContainer, Alloc>::value_type&
ual_vertex_value<EV, VV, GV, VId, VContainer, Alloc>::value() noexcept {
  return value_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr const typename ual_vertex_value<EV, VV, GV, VId, VContainer, Alloc>::value_type&
ual_vertex_value<EV, VV, GV, VId, VContainer, Alloc>::value() const noexcept {
  return value_;
}

///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list::const_iterator (new constructors and graph/source_id)
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::const_iterator(
      const graph_type& g, vertex_id_type uid, const edge_type* uv) noexcept
      : vertex_id_(uid), edge_(const_cast<edge_type*>(uv)), graph_(const_cast<graph_type*>(&g)) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::graph_type&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::graph() {
  return *graph_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
const typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::graph_type&
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::graph() const {
  return *graph_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_id_type
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::const_iterator::source_id() const {
  return vertex_id_;
}

///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list::iterator (new constructor)
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::iterator::iterator(
      const graph_type& g, vertex_id_type uid, const edge_type* uv)
      : const_iterator(g, uid, uv) {}

///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list move constructor
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
ual_vertex_edge_list<EV, VV, GV, VId, VContainer, Alloc>::ual_vertex_edge_list(
      ual_vertex_edge_list&& rhs) noexcept
      : head_(move(rhs.head_)), tail_(move(rhs.tail_)), size_(move(rhs.size_)) {
  rhs.head_ = rhs.tail_ = nullptr;
  rhs.size_             = 0;
}

///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list_link
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::ual_vertex_edge_list_link(
      vertex_id_type uid) noexcept
      : vertex_id_(uid) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
typename ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::vertex_id_type
ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::vertex_id() const noexcept {
  return vertex_id_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
typename ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::const_vertex_iterator
ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::vertex(const graph_type& g) const {
  return g.vertices().begin() + vertex_id_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
typename ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::vertex_iterator
ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::vertex(graph_type& g) {
  return g.vertices().begin() + vertex_id_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
typename ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::edge_type*
ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::next() noexcept {
  return next_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
const typename ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::edge_type*
ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::next() const noexcept {
  return next_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
typename ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::edge_type*
ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::prev() noexcept {
  return prev_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename ListT>
const typename ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::edge_type*
ual_vertex_edge_list_link<EV, VV, GV, VId, VContainer, Alloc, ListT>::prev() const noexcept {
  return prev_;
}

///-------------------------------------------------------------------------------------
/// ual_edge::renumber_after_vertex_erase
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void ual_edge<EV, VV, GV, VId, VContainer, Alloc>::renumber_after_vertex_erase(
      vertex_id_type removed_id) noexcept {
  auto& in_link  = static_cast<vertex_edge_list_inward_link_type&>(*this);
  auto& out_link = static_cast<vertex_edge_list_outward_link_type&>(*this);
  if (in_link.vertex_id_ > removed_id)
    --in_link.vertex_id_;
  if (out_link.vertex_id_ > removed_id)
    --out_link.vertex_id_;
}

///-------------------------------------------------------------------------------------
/// base_undirected_adjacency_list::const_edge_iterator
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::const_edge_iterator(
      const graph_type& g, vertex_iterator u)
      : g_(&const_cast<graph_type&>(g)), u_(u) {
  advance_vertex();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::const_edge_iterator(
      const graph_type& g, const_vertex_iterator u)
      : g_(&const_cast<graph_type&>(g)),
        u_(g_->vertices().begin() + (u - g.vertices().begin())) {
  advance_vertex();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::const_edge_iterator(
      const graph_type& g, vertex_iterator u, vertex_edge_iterator uv)
      : g_(&const_cast<graph_type&>(g)), u_(u), uv_(uv) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::reference
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::operator*() const {
  return *uv_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::pointer
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::operator->() const {
  return &*uv_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator&
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::operator++() {
  advance_edge();
  return *this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::operator++(int) {
  const_edge_iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
bool base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::operator==(
      const const_edge_iterator& rhs) const noexcept {
  return uv_ == rhs.uv_ && u_ == rhs.u_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
bool base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::operator!=(
      const const_edge_iterator& rhs) const noexcept {
  return !operator==(rhs);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::advance_edge() {
  vertex_id_type uid = static_cast<vertex_id_type>(u_ - g_->vertices().begin());
  if (++uv_ != u_->edges_end(*g_, uid))
    return;
  ++u_;
  advance_vertex();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_iterator::advance_vertex() {
  for (; u_ != g_->vertices().end(); ++u_) {
    if (u_->num_edges() > 0) {
      vertex_id_type uid = static_cast<vertex_id_type>(u_ - g_->vertices().begin());
      uv_                = u_->edges_begin(*g_, uid);
      return;
    }
  }
}

///-------------------------------------------------------------------------------------
/// base_undirected_adjacency_list::edge_iterator
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::edge_iterator(
      graph_type& g, vertex_iterator u) noexcept
      : const_edge_iterator(g, u) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::edge_iterator(
      graph_type& g, vertex_iterator u, vertex_edge_iterator uv)
      : const_edge_iterator(g, u, uv) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::edge_iterator() noexcept
      : const_edge_iterator() {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::edge_iterator(
      const edge_iterator& rhs) noexcept
      : const_edge_iterator(rhs) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::edge_iterator(
      const_edge_iterator& rhs)
      : const_edge_iterator(rhs) {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::~edge_iterator() {}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator&
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::operator=(
      const edge_iterator& rhs) noexcept {
  const_edge_iterator::operator=(rhs);
  return *this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::reference
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::operator*() const {
  return *this->uv_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::pointer
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::operator->() const {
  return &*this->uv_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator&
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::operator++() {
  this->advance_edge();
  return *this;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_iterator::operator++(int) {
  edge_iterator tmp(*this);
  ++*this;
  return tmp;
}

///-------------------------------------------------------------------------------------
/// base_undirected_adjacency_list: num_vertices, num_edges, has_edge, edges
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr auto
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::num_vertices() const noexcept {
  return vertices_.size();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_size_type
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::num_edges() const noexcept {
  return edges_size_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr bool
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::has_edge() const noexcept {
  return edges_size_ > 0;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edge_range
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edges() {
  auto& self = static_cast<graph_type&>(*this);
  return {edge_iterator(self, begin()), edge_iterator(self, end()), this->edges_size_};
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::const_edge_range
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::edges() const {
  auto& self = static_cast<const graph_type&>(*this);
  return {const_edge_iterator(self, const_cast<graph_type&>(self).begin()),
          const_edge_iterator(self, const_cast<graph_type&>(self).end()), this->edges_size_};
}

///-------------------------------------------------------------------------------------
/// base_undirected_adjacency_list: add_vertex
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_vertex() {
  this->vertices_.push_back(
        vertex_type(this->vertices_, static_cast<vertex_id_type>(this->vertices_.size())));
  return this->vertices_.begin() + static_cast<vertex_difference_type>(this->vertices_.size() - 1);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_vertex(vertex_value_type&& val) {
  this->vertices_.push_back(
        vertex_type(this->vertices_, static_cast<vertex_id_type>(this->vertices_.size()), std::move(val)));
  return this->vertices_.begin() + static_cast<vertex_difference_type>(this->vertices_.size() - 1);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
template <class VV2>
requires std::constructible_from<VV, const VV2&>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_vertex(const VV2& val) {
  this->vertices_.push_back(
        vertex_type(this->vertices_, static_cast<vertex_id_type>(this->vertices_.size()), val));
  return this->vertices_.begin() + static_cast<vertex_id_type>(this->vertices_.size() - 1);
}

///-------------------------------------------------------------------------------------
/// base_undirected_adjacency_list: add_edge
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_edge(vertex_id_type uid,
                                                                              vertex_id_type vid) {
  vertex_iterator ui = try_find_vertex(uid);
  vertex_iterator vi = try_find_vertex(vid);
  return add_edge(ui, vi);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_edge(vertex_id_type   uid,
                                                                              vertex_id_type   vid,
                                                                              edge_value_type&& val) {
  vertex_iterator ui = this->vertices_.begin() + uid;
  vertex_iterator vi = this->vertices_.begin() + vid;
  return add_edge(ui, vi, std::move(val));
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
template <class EV2>
requires std::constructible_from<EV, const EV2&>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_edge(vertex_id_type uid,
                                                                              vertex_id_type vid,
                                                                              const EV2&     val) {
  vertex_iterator ui = this->vertices_.begin() + uid;
  vertex_iterator vi = this->vertices_.begin() + vid;
  return add_edge(ui, vi, val);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_edge(vertex_iterator u,
                                                                              vertex_iterator v) {
  vertex_id_type uid = static_cast<vertex_id_type>(u - this->vertices_.begin());
  edge_type*     uv  = this->edge_alloc_.allocate(1);
  new (uv) edge_type(static_cast<graph_type&>(*this), u, v);
  ++this->edges_size_;
  return vertex_edge_iterator(static_cast<graph_type&>(*this), uid, uv);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_edge(vertex_iterator   u,
                                                                              vertex_iterator   v,
                                                                              edge_value_type&& val) {
  vertex_id_type uid = static_cast<vertex_id_type>(u - this->vertices_.begin());
  edge_type*     uv  = this->edge_alloc_.allocate(1);
  new (uv) edge_type(static_cast<graph_type&>(*this), u, v, std::move(val));
  ++this->edges_size_;
  return vertex_edge_iterator(static_cast<graph_type&>(*this), uid, uv);
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
template <class EV2>
requires std::constructible_from<EV, const EV2&>
typename base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::add_edge(vertex_iterator u,
                                                                              vertex_iterator v,
                                                                              const EV2&      val) {
  vertex_id_type uid = static_cast<vertex_id_type>(u - this->vertices_.begin());
  edge_type*     uv  = this->edge_alloc_.allocate(1);
  new (uv) edge_type(static_cast<graph_type&>(*this), u, v, val);
  ++this->edges_size_;
  return vertex_edge_iterator(static_cast<graph_type&>(*this), uid, uv);
}

///-------------------------------------------------------------------------------------
/// undirected_adjacency_list<EV, VV, GV, ...>::graph_value (GV != void)
///

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
typename undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::graph_value_type&
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::graph_value() noexcept {
  return graph_value_;
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
const typename undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::graph_value_type&
undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>::graph_value() const noexcept {
  return graph_value_;
}

///-------------------------------------------------------------------------------------
/// CPO non-member free functions (find_vertex, target_id, source_id, edge_value)
///
/// These are defined here (in impl.hpp) so that they are available to all code
/// that includes undirected_adjacency_list.hpp (which #includes this file).
///-------------------------------------------------------------------------------------

/// find_vertex(g, id) - returns view iterator yielding vertex_descriptor
/// REQUIRED: Provides bounds checking - returns end() if id >= size()
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr auto find_vertex(undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>& g, VId id) noexcept {
  using graph_type     = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_set     = typename graph_type::vertex_set;
  using container_iter = typename vertex_set::iterator;
  using view_type      = vertex_descriptor_view<container_iter>;
  using view_iterator  = typename view_type::iterator;
  using storage_type   = typename view_iterator::value_type::storage_type;

  if (id >= static_cast<VId>(g.vertices().size())) {
    return view_iterator{static_cast<storage_type>(g.vertices().size())};
  }
  return view_iterator{static_cast<storage_type>(id)};
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc>
constexpr auto find_vertex(const undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>& g, VId id) noexcept {
  using graph_type     = undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>;
  using vertex_set     = typename graph_type::vertex_set;
  using container_iter = typename vertex_set::const_iterator;
  using view_type      = vertex_descriptor_view<container_iter>;
  using view_iterator  = typename view_type::iterator;
  using storage_type   = typename view_iterator::value_type::storage_type;

  if (id >= static_cast<VId>(g.vertices().size())) {
    return view_iterator{static_cast<storage_type>(g.vertices().size())};
  }
  return view_iterator{static_cast<storage_type>(id)};
}

/// target_id(g, edge_descriptor) - get target vertex id from edge descriptor (iteration perspective)
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename E>
requires edge_descriptor_type<E>
constexpr VId target_id(const undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>& g,
                        const E&                                                               e) noexcept {
  return e.value()->other_vertex_id(g, static_cast<VId>(e.source_id()));
}

/// source_id(g, edge_descriptor) - get source vertex id from edge descriptor (iteration perspective)
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename E>
requires edge_descriptor_type<E>
constexpr VId source_id([[maybe_unused]] const undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>& g,
                        const E& e) noexcept {
  return static_cast<VId>(e.source_id());
}

/// edge_value(g, edge_descriptor) - get edge value from edge descriptor (non-void EV only)
template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename E>
requires edge_descriptor_type<E> && (!std::is_void_v<EV>)
constexpr decltype(auto)
      edge_value(undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>&, const E& e) noexcept {
  return e.value()->value();
}

template <typename EV,
          typename VV,
          typename GV,
          integral VId,
          template <typename V, typename A> class VContainer,
          typename Alloc,
          typename E>
requires edge_descriptor_type<E> && (!std::is_void_v<EV>)
constexpr decltype(auto)
      edge_value(const undirected_adjacency_list<EV, VV, GV, VId, VContainer, Alloc>&, const E& e) noexcept {
  return e.value()->value();
}

} // namespace graph::container

#endif // UNDIRECTED_ADJ_LIST_IMPL_HPP
