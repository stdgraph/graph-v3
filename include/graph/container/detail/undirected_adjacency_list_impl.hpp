//
//	Author: J. Phillip Ratzloff
//

#ifndef UNDIRECTED_ADJ_LIST_IMPL_HPP
#define UNDIRECTED_ADJ_LIST_IMPL_HPP

#include <stdexcept>

namespace graph::container {

///-------------------------------------------------------------------------------------
/// ual_vertex_edge_list
///

// ual_vertex_edge_list::const_iterator
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::reference
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::operator*() const {
  return *edge_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::pointer
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::operator->() const {
  return edge_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator&
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::operator++() {
  advance();
  return *this;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::operator++(int) {
  const_iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator&
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::operator--() {
  retreat();
  return *this;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::operator--(int) {
  const_iterator tmp(*this);
  --*this;
  return tmp;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::advance() {
  edge_type* start_edge = edge_; // Remember where we started for cycle detection
  
  vertex_edge_list_inward_link_type&  inward_link  = *edge_; // in.vertex_key_ == this->vertex_key_;
  vertex_edge_list_outward_link_type& outward_link = *edge_;
  if (inward_link.vertex_key_ == vertex_key_) {
    edge_ = inward_link.next_;
  } else {
    assert(outward_link.vertex_key_ == vertex_key_);
    edge_ = outward_link.next_;
  }
  
  // Self-loop detection: if we've cycled back to the starting edge, treat as end
  if (edge_ == start_edge) {
    edge_ = nullptr;
  }
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::retreat() {
  if (edge_) {
    vertex_edge_list_inward_link_type&  inward_link  = *edge_; // in.vertex_key_ == this->vertex_key_;
    vertex_edge_list_outward_link_type& outward_link = *edge_;
    if (inward_link.vertex_key_ == vertex_key_) {
      edge_ = inward_link.prev_;
    } else {
      assert(outward_link.vertex_key_ == vertex_key_);
      edge_ = outward_link.prev_;
    }
  } else {
    vertex_iterator u = graph_->try_find_vertex(vertex_key_);
    edge_             = &u->edge_back(*graph_);
  }
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
bool ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::operator==(
      const const_iterator& rhs) const noexcept {
  return edge_ == rhs.edge_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
bool ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator::operator!=(
      const const_iterator& rhs) const noexcept {
  return !(*this == rhs);
}

// ual_vertex_edge_list::iterator
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::reference
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator::operator*() const {
  return *this->edge_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::pointer
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator::operator->() const {
  return this->edge_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator&
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator::operator++() {
  this->advance();
  return *this;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator::operator++(int) {
  iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator&
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator::operator--() {
  this->retreat();
  return *this;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator::operator--(int) {
  iterator tmp(*this);
  --*this;
  return tmp;
}

// ual_vertex_edge_list
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::size_type
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::size() const noexcept {
  return size_;
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
bool ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::empty() const noexcept {
  return size_ == 0;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::front() {
  return *head_;
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
const typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::front() const {
  return *head_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::back() {
  return *tail_;
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
const typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::back() const {
  return *tail_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <typename ListT>
void ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::link_front(
      edge_type& uv, ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, ListT>& uv_link) {
  using link_t = ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, ListT>;
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

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <typename ListT>
void ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::link_back(
      edge_type& uv, ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, ListT>& uv_link) {
  if (tail_) {
    vertex_edge_list_inward_link_type&  tail_inward_link  = static_cast<vertex_edge_list_inward_link_type&>(*tail_);
    vertex_edge_list_outward_link_type& tail_outward_link = static_cast<vertex_edge_list_outward_link_type&>(*tail_);
    if (tail_inward_link.vertex_key_ == uv_link.vertex_key_) {
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

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <typename ListT>
void ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::unlink(
      edge_type& uv, ual_vertex_edge_list_link<VV, EV, GV, VId, VContainer, Alloc, ListT>& uv_link) {

  if (uv_link.prev_) {
    vertex_edge_list_inward_link_type& prev_inward_link =
          static_cast<vertex_edge_list_inward_link_type&>(*uv_link.prev_);
    vertex_edge_list_outward_link_type& prev_outward_link =
          static_cast<vertex_edge_list_outward_link_type&>(*uv_link.prev_);
    if (prev_inward_link.vertex_key_ == uv_link.vertex_key_) {
      prev_inward_link.next_ = uv_link.next_;
    } else {
      assert(prev_outward_link.vertex_key_ == uv_link.vertex_key_);
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
    if (next_inward_link.vertex_key_ == uv_link.vertex_key_) {
      next_inward_link.prev_ = uv_link.prev_;
    } else {
      assert(next_outward_link.vertex_key_ == uv_link.vertex_key_);
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

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::begin(graph_type& g, vertex_key_type ukey) noexcept {
  return iterator(g, ukey, head_);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::begin(const graph_type& g,
                                                                 vertex_key_type   ukey) const noexcept {
  return const_iterator(g, ukey, head_);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::cbegin(const graph_type& g,
                                                                  vertex_key_type   ukey) const noexcept {
  return const_iterator(g, ukey, head_);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::end(graph_type& g, vertex_key_type ukey) noexcept {
  return iterator(g, ukey, nullptr);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::end(const graph_type& g,
                                                               vertex_key_type   ukey) const noexcept {
  return const_iterator(g, ukey, nullptr);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_iterator
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::cend(const graph_type& g,
                                                                vertex_key_type   ukey) const noexcept {
  return const_iterator(g, ukey, nullptr);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::edge_range
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::edges(graph_type& g, vertex_key_type ukey) noexcept {
  return {iterator(g, ukey, head_), iterator(g, ukey, nullptr), size_};
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::const_edge_range
ual_vertex_edge_list<VV, EV, GV, VId, VContainer, Alloc>::edges(const graph_type& g,
                                                                 vertex_key_type   ukey) const noexcept {
  return {const_iterator(g, ukey, head_), const_iterator(g, ukey, nullptr), size_};
}

///-------------------------------------------------------------------------------------
/// ual_edge
///
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&     g,
                                                        vertex_key_type ukey,
                                                        vertex_key_type vkey) noexcept
      : base_value_type(), vertex_edge_list_inward_link_type(ukey), vertex_edge_list_outward_link_type(vkey) {
  link_back(g.vertices()[ukey], g.vertices()[vkey]);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&            g,
                                                        vertex_key_type        ukey,
                                                        vertex_key_type        vkey,
                                                        const edge_value_type& val) noexcept
      : base_value_type(val), vertex_edge_list_inward_link_type(ukey), vertex_edge_list_outward_link_type(vkey) {
  link_back(g.vertices()[ukey], g.vertices()[vkey]);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&       g,
                                                        vertex_key_type   ukey,
                                                        vertex_key_type   vkey,
                                                        edge_value_type&& val) noexcept
      : base_value_type(move(val)), vertex_edge_list_inward_link_type(ukey), vertex_edge_list_outward_link_type(vkey) {
  link_back(g.vertices()[ukey], g.vertices()[vkey]);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::ual_edge(graph_type& g, vertex_iterator ui, vertex_iterator vi) noexcept
      : base_value_type()
      , vertex_edge_list_inward_link_type(vertex_key(g, ui))
      , vertex_edge_list_outward_link_type(vertex_key(g, vi)) {
  link_back(*ui, *vi);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&            g,
                                                        vertex_iterator        ui,
                                                        vertex_iterator        vi,
                                                        const edge_value_type& val) noexcept
      : base_value_type(val)
      , vertex_edge_list_inward_link_type(vertex_key(g, ui))
      , vertex_edge_list_outward_link_type(vertex_key(g, vi)) {
  link_back(*ui, *vi);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::ual_edge(graph_type&       g,
                                                        vertex_iterator   ui,
                                                        vertex_iterator   vi,
                                                        edge_value_type&& val) noexcept
      : base_value_type(move(val))
      , vertex_edge_list_inward_link_type(vertex_key(g, ui))
      , vertex_edge_list_outward_link_type(vertex_key(g, vi)) {
  link_back(*ui, *vi);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::~ual_edge() noexcept {
  vertex_edge_list_outward_link_type& outward_link = *static_cast<vertex_edge_list_outward_link_type*>(this);
  assert(outward_link.prev() == nullptr && outward_link.next() == nullptr); // has edge been unlinked?

  vertex_edge_list_inward_link_type& inward_link = *static_cast<vertex_edge_list_inward_link_type*>(this);
  assert(inward_link.prev() == nullptr && inward_link.next() == nullptr); // has edge been unlinked?
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void ual_edge<VV, EV, GV, VId, VContainer, Alloc>::link_front(vertex_type& u, vertex_type& v) noexcept {
  u.edges_.link_front(*this, *static_cast<vertex_edge_list_inward_link_type*>(this));
  v.edges_.link_front(*this, *static_cast<vertex_edge_list_outward_link_type*>(this));
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void ual_edge<VV, EV, GV, VId, VContainer, Alloc>::link_back(vertex_type& u, vertex_type& v) noexcept {
  u.edges_.link_back(*this, *static_cast<vertex_edge_list_inward_link_type*>(this));
  v.edges_.link_back(*this, *static_cast<vertex_edge_list_outward_link_type*>(this));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void ual_edge<VV, EV, GV, VId, VContainer, Alloc>::unlink(vertex_type& u, vertex_type& v) noexcept {
  u.edges_.unlink(*this, *static_cast<vertex_edge_list_inward_link_type*>(this));
  v.edges_.unlink(*this, *static_cast<vertex_edge_list_outward_link_type*>(this));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::source_vertex(graph_type& g) noexcept {
  return g.vertices().begin() + source_vertex_key(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::source_vertex(const graph_type& g) const noexcept {
  return static_cast<vertex_edge_list_inward_link_type const*>(this)->vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::vertex_key_type
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::source_vertex_key([[maybe_unused]] const graph_type& g) const noexcept {
  return static_cast<vertex_edge_list_inward_link_type const*>(this)->vertex_key();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::target_vertex(graph_type& g) noexcept {
  return g.vertices().begin() + target_vertex_key(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::target_vertex(const graph_type& g) const noexcept {
  return static_cast<vertex_edge_list_outward_link_type const*>(this)->vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::vertex_key_type
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::target_vertex_key([[maybe_unused]] const graph_type& g) const noexcept {
  return static_cast<vertex_edge_list_outward_link_type const*>(this)->vertex_key();
}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::other_vertex(graph_type& g, const_vertex_iterator other) noexcept {
  return other != source_vertex(g) ? source_vertex(g) : target_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::other_vertex(const graph_type&     g,
                                                            const_vertex_iterator other) const noexcept {
  return other != source_vertex(g) ? source_vertex(g) : target_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::other_vertex(graph_type& g, vertex_key_type other_key) noexcept {
  return other_key != source_vertex_key(g) ? source_vertex(g) : target_vertex(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::other_vertex(const graph_type& g,
                                                            vertex_key_type   other_key) const noexcept {
  return other_key != source_vertex_key(g) ? source_vertex(g) : target_vertex(g);
}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::vertex_key_type
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::other_vertex_key(const graph_type&     g,
                                                                const_vertex_iterator other) const noexcept {
  return other != source_vertex(g) ? source_vertex_key(g) : target_vertex_key(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::vertex_key_type
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::other_vertex_key(const graph_type& g,
                                                                vertex_key_type   other_key) const noexcept {
  return other_key != source_vertex_key(g) ? source_vertex_key(g) : target_vertex_key(g);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_edge<VV, EV, GV, VId, VContainer, Alloc>::edge_key_type
ual_edge<VV, EV, GV, VId, VContainer, Alloc>::edge_key(const graph_type& g) const noexcept {
  return unordered_pair(source_vertex_key(g), target_vertex_key(g));
}


///-------------------------------------------------------------------------------------
/// ual_vertex
///
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::ual_vertex([[maybe_unused]] vertex_set& vertices, [[maybe_unused]] vertex_index index) {}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::ual_vertex([[maybe_unused]] vertex_set& vertices,
                                                            [[maybe_unused]] vertex_index index,
                                                            const vertex_value_type& val)
      : base_value_type(val) {}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::ual_vertex([[maybe_unused]] vertex_set& vertices,
                                                            [[maybe_unused]] vertex_index index,
                                                            vertex_value_type&& val) noexcept
      : base_value_type(move(val)) {}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges_begin(graph_type& g, vertex_key_type ukey) noexcept {
  return edges_.begin(g, ukey);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges_begin(const graph_type& g, vertex_key_type ukey) const noexcept {
  return edges_.begin(g, ukey);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges_cbegin(const graph_type& g,
                                                              vertex_key_type   ukey) const noexcept {
  return edges_.cbegin(g, ukey);
}

// Removed: e_begin implementation - legacy method using const_cast, replaced by edges_begin


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges_end(graph_type& g, vertex_key_type ukey) noexcept {
  return edges_.end(g, ukey);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges_end(const graph_type& g, vertex_key_type ukey) const noexcept {
  return edges_.end(g, ukey);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges_cend(const graph_type& g, vertex_key_type ukey) const noexcept {
  return edges_.cend(g, ukey);
}

// Removed: e_end implementation - legacy method using const_cast, replaced by edges_end

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_range
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges(graph_type& g, vertex_key_type ukey) {
  return edges_.edges(g, ukey);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_edge_range
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges(const graph_type& g, vertex_key_type ukey) const {
  return edges_.edges(g, ukey);
}


#if 0
template <typename VV, typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_key_type
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_key(const graph_type& g) const noexcept {
  return static_cast<vertex_key_type>(this - g.vertices().data());
}
#endif


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edge_front(graph_type& g) noexcept {
  return edges_.front(g, *this);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
const typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edge_front(const graph_type& g) const noexcept {
  return edges_.front(g, *this);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edge_back(graph_type& g) noexcept {
  return edges_.back();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
const typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edge_type&
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edge_back(const graph_type& g) const noexcept {
  return edges_.back();
}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_size_type
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::edges_size() const {
  return edges_.size();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::erase_edge(graph_type& g, edge_type* uv) {
  vertex_type& u = g.vertices()[uv->source_vertex_key(g)];
  vertex_type& v = g.vertices()[uv->target_vertex_key(g)];
  uv->unlink(u, v);

  uv->~edge_type();
  g.edge_alloc_.deallocate(uv, 1);
  --g.edges_size_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::clear_edges(graph_type& g) {
  while (!edges_.empty()) {
    erase_edge(g, &edges_.front());
  }
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::erase_edge(graph_type& g, vertex_edge_iterator uvi) {
  edge_type* uv = &*uvi;
  ++uvi;
  erase_edge(g, uv);
  return uvi;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::erase_edge(graph_type&          g,
                                                            vertex_edge_iterator first,
                                                            vertex_edge_iterator last) {
  while (first != last)
    first = erase_edge(g, first);
  return first;
}

#if 0
template <typename VV, typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::create_edge(graph_type& g, vertex_type& v) {
  edge_type* uv = g.edge_alloc_.allocate(1);
  new (uv) edge_type(g, *this, v);
  ++g.edges_size_;
  return vertex_edge_iterator(g, *this, uv);
}

template <typename VV, typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::create_edge(graph_type& g, vertex_type& v, edge_value_type&& val) {
  edge_type* uv = g.edge_alloc_.allocate(1);
  new (uv) edge_type(g, *this, v, move(val));
  ++g.edges_size_;
  return vertex_edge_iterator(g, *this, uv);
}

template <typename VV, typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::create_edge(graph_type& g, vertex_type& v, const edge_value_type& val) {
  edge_type* uv = g.edge_alloc_.allocate(1);
  new (uv) edge_type(g, *this, v, val);
  ++g.edges_size_;
  return vertex_edge_iterator(g, *this, uv);
}
#endif

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_vertex_size_type
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertices_size(const graph_type& g) const {
  return size(edges(g));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertex_vertex_range
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertices(graph_type& g, vertex_key_type ukey) {
  return {vertex_vertex_iterator(edges_.begin(g, ukey)), vertex_vertex_iterator(edges_.end(g, ukey)), edges_.size()};
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_vertex_range
ual_vertex<VV, EV, GV, VId, VContainer, Alloc>::vertices(const graph_type& g, vertex_key_type ukey) const {
  return {const_vertex_vertex_iterator(edges_.begin(g, ukey)), const_vertex_vertex_iterator(edges_.end(g, ukey)),
          edges_.size()};
}


///-------------------------------------------------------------------------------------
/// base_undirected_adjacency_list
///

// Copy constructor - copies vertices and edges
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::base_undirected_adjacency_list(
      const base_undirected_adjacency_list& other) {
  // Copy base class members
  edge_alloc_ = other.edge_alloc_;
  
  // Reserve space and copy vertices (with empty edge lists)
  vertices_.reserve(other.vertices_.size());
  
  for (const auto& v : other.vertices_) {
    if constexpr (std::is_void_v<VV>) {
      vertices_.push_back(vertex_type(vertices_, static_cast<vertex_key_type>(vertices_.size())));
    } else {
      // Access vertex value member directly
      vertices_.push_back(vertex_type(vertices_, static_cast<vertex_key_type>(vertices_.size()), v.value));
    }
  }
  
  // Downcast to graph_type to access methods
  auto& g = static_cast<graph_type&>(*this);
  
  // Copy edges - iterate through each vertex and copy edges where source_key <= target_key to avoid duplicates
  for (vertex_key_type ukey = 0; ukey < static_cast<vertex_key_type>(other.vertices_.size()); ++ukey) {
    const auto& src_vtx = other.vertices_[ukey];
    for (auto uv = src_vtx.edges_begin(static_cast<const graph_type&>(other), ukey); 
         uv != src_vtx.edges_end(static_cast<const graph_type&>(other), ukey); ++uv) {
      vertex_key_type src_key = uv->source_vertex_key(static_cast<const graph_type&>(other));
      vertex_key_type tgt_key = uv->target_vertex_key(static_cast<const graph_type&>(other));
      // Only copy each edge once: when ukey matches source and source <= target
      if (ukey == src_key && src_key <= tgt_key) {
        if constexpr (std::is_void_v<EV>) {
          g.create_edge(src_key, tgt_key);
        } else {
          g.create_edge(src_key, tgt_key, uv->value);
        }
      }
    }
  }
}

// Range constructor
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <typename ERng, typename VRng, typename EProj, typename VProj>
  requires ranges::forward_range<ERng>
        && ranges::input_range<VRng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::base_undirected_adjacency_list(
      const ERng& erng, const VRng& vrng, 
      const EProj& eproj, const VProj& vproj, 
      const allocator_type& alloc)
      : vertices_(alloc)
      , edge_alloc_(alloc) {
  // Handle empty case - no vertices or edges to create
  if (vrng.empty() && ranges::empty(erng)) {
    return;
  }

  // Evaluate max vertex key needed
  vertex_key_type max_vtx_key = vrng.empty() ? vertex_key_type(0) 
                                             : static_cast<vertex_key_type>(vrng.size() - 1);
  for (auto& e : erng) {
    auto&& edge_info = eproj(e);  // copyable_edge_t<VId, EV>
    max_vtx_key = max(max_vtx_key, max(edge_info.source_id, edge_info.target_id));
  }

  // add vertices
  vertices_.reserve(max_vtx_key + 1);
  if constexpr (!std::is_void_v<VV>) {
    for (auto& vtx : vrng) {
      auto&& vtx_info = vproj(vtx); // copyable_vertex_t<VId, VV>
      vertices_.push_back(vertex_type(vertices_, static_cast<vertex_key_type>(vertices_.size()), vtx_info.value));
    }
  }
  vertices_.resize(max_vtx_key + 1); // assure expected vertices exist

  // Downcast to graph_type to access create_edge
  auto& g = static_cast<graph_type&>(*this);
  
  // add edges
  if (!ranges::empty(erng)) {
    auto&& first_edge_info = eproj(*ranges::begin(erng)); // first edge
    vertex_key_type tkey = first_edge_info.source_id;     // last in-vertex key
    for (auto& edge_data : erng) {
      auto&& edge_info = eproj(edge_data); // copyable_edge_t<VId, EV>
      if (edge_info.source_id < tkey)
        g.throw_unordered_edges();

      if constexpr (std::is_void_v<EV>) {
        g.create_edge(edge_info.source_id, edge_info.target_id);
      } else {
        g.create_edge(edge_info.source_id, edge_info.target_id, edge_info.value);
      }
      tkey = edge_info.source_id;
    }
  }
}

// Initializer list constructor with edge values
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::base_undirected_adjacency_list(
      const initializer_list<tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist, 
      const allocator_type& alloc)
      : vertices_(alloc)
      , edge_alloc_(alloc) {
  // Evaluate max vertex key needed
  vertex_key_type max_vtx_key = vertex_key_type();
  for (auto& edge_data : ilist) {
    const auto& [ukey, vkey, uv_val] = edge_data;
    max_vtx_key = max(max_vtx_key, max(ukey, vkey));
  }
  vertices_.resize(max_vtx_key + 1); // assure expected vertices exist

  // Downcast to graph_type to access create_edge and throw_unordered_edges
  auto& g = static_cast<graph_type&>(*this);
  
  // add edges
  if (ilist.size() > 0) {
    auto [tkey, uukey, tu_val] = *ranges::begin(ilist);
    for (auto& edge_data : ilist) {
      const auto& [ukey, vkey, uv_val] = edge_data;
      if (ukey < tkey)
        g.throw_unordered_edges();

      g.create_edge(ukey, vkey, uv_val);
      tkey = ukey;
    }
  }
}

// Initializer list constructor without edge values
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::base_undirected_adjacency_list(
      const initializer_list<tuple<vertex_key_type, vertex_key_type>>& ilist, 
      const allocator_type& alloc)
      : vertices_(alloc)
      , edge_alloc_(alloc) {
  // Evaluate max vertex key needed
  vertex_key_type max_vtx_key = vertex_key_type();
  for (auto& edge_data : ilist) {
    const auto& [ukey, vkey] = edge_data;
    max_vtx_key = max(max_vtx_key, max(ukey, vkey));
  }
  vertices_.resize(max_vtx_key + 1); // assure expected vertices exist

  // Downcast to graph_type to access create_edge and throw_unordered_edges
  auto& g = static_cast<graph_type&>(*this);
  
  // add edges
  if (ilist.size() > 0) {
    auto [tkey, uukey] = *ranges::begin(ilist);
    for (auto& edge_data : ilist) {
      const auto& [ukey, vkey] = edge_data;
      if (ukey < tkey)
        g.throw_unordered_edges();

      g.create_edge(ukey, vkey);
      tkey = ukey;
    }
  }
}


// Destructor
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::~base_undirected_adjacency_list() {
  // Downcast to graph_type to access clear() method
  auto& g = static_cast<graph_type&>(*this);
  g.clear(); // assure edges are deleted using edge_alloc_
}

// Copy assignment operator
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>& 
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::operator=(
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
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::throw_unordered_edges() const {
  assert(false); // container must be sorted by edge_key.first
  throw std::invalid_argument("edges not ordered");
}

//-------------------------------------------------------------------------------------
// Accessor methods
//-------------------------------------------------------------------------------------

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::edge_allocator_type
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::edge_allocator() const noexcept {
  return this->edge_alloc_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_set&
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertices() {
  return this->vertices_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr const typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_set&
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertices() const {
  return this->vertices_;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::begin() {
  return this->vertices_.begin();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::begin() const {
  return this->vertices_.begin();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::cbegin() const {
  return this->vertices_.cbegin();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::end() {
  return this->vertices_.end();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::end() const {
  return this->vertices_.end();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::cend() const {
  return this->vertices_.cend();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::try_find_vertex(vertex_key_type key) {
  if (key < this->vertices_.size())
    return this->vertices_.begin() + key;
  else
    return this->vertices_.end();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::try_find_vertex(vertex_key_type key) const {
  if (key < this->vertices_.size())
    return this->vertices_.begin() + key;
  else
    return this->vertices_.end();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::edge_size_type
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::edges_size() const noexcept {
  return this->edges_size_;
}

//-------------------------------------------------------------------------------------
// Vertex creation methods
//-------------------------------------------------------------------------------------

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_vertex() {
  this->vertices_.push_back(vertex_type(this->vertices_, static_cast<vertex_key_type>(this->vertices_.size())));
  return this->vertices_.begin() + static_cast<vertex_difference_type>(this->vertices_.size() - 1);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_vertex(vertex_value_type&& val) {
  this->vertices_.push_back(vertex_type(this->vertices_, static_cast<vertex_key_type>(this->vertices_.size()), move(val)));
  return this->vertices_.begin() + static_cast<vertex_difference_type>(this->vertices_.size() - 1);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <class VV2>
  requires std::constructible_from<typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_value_type, const VV2&>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_vertex(const VV2& val) {
  this->vertices_.push_back(vertex_type(this->vertices_, static_cast<vertex_key_type>(this->vertices_.size()), val));
  return this->vertices_.begin() + static_cast<vertex_key_type>(this->vertices_.size() - 1);
}

//-------------------------------------------------------------------------------------
// Edge creation methods
//-------------------------------------------------------------------------------------

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_edge(vertex_key_type from_key,
                                                                                  vertex_key_type to_key) {
  vertex_iterator ui = try_find_vertex(from_key);
  vertex_iterator vi = try_find_vertex(to_key);
  return create_edge(ui, vi);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_edge(vertex_key_type   from_key,
                                                                                  vertex_key_type   to_key,
                                                                                  edge_value_type&& val) {
  vertex_iterator ui = this->vertices_.begin() + from_key;
  vertex_iterator vi = this->vertices_.begin() + to_key;
  return create_edge(ui, vi, move(val));
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <class EV2>
  requires std::constructible_from<typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::edge_value_type, const EV2&>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_edge(vertex_key_type from_key,
                                                                                  vertex_key_type to_key,
                                                                                  const EV2&      val) {
  vertex_iterator ui = this->vertices_.begin() + from_key;
  vertex_iterator vi = this->vertices_.begin() + to_key;
  return create_edge(ui, vi, val);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_edge(vertex_iterator u, vertex_iterator v) {
  vertex_key_type ukey = u - this->vertices_.begin();
  edge_type*      uv   = this->edge_alloc_.allocate(1);
  new (uv) edge_type(static_cast<graph_type&>(*this), u, v);
  ++this->edges_size_;
  return vertex_edge_iterator(static_cast<graph_type&>(*this), ukey, uv);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_edge(vertex_iterator   u,
                                                                                  vertex_iterator   v,
                                                                                  edge_value_type&& val) {
  vertex_key_type ukey = vertex_key(static_cast<graph_type&>(*this), u);
  edge_type*      uv   = this->edge_alloc_.allocate(1);
  new (uv) edge_type(static_cast<graph_type&>(*this), u, v, move(val));
  ++this->edges_size_;
  return vertex_edge_iterator(static_cast<graph_type&>(*this), ukey, uv);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <class EV2>
  requires std::constructible_from<typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::edge_value_type, const EV2&>
typename base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::vertex_edge_iterator
base_undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::create_edge(vertex_iterator u,
                                                                                  vertex_iterator v,
                                                                                  const EV2&      val) {
  vertex_key_type ukey = vertex_key(static_cast<graph_type&>(*this), u);
  edge_type*      uv   = this->edge_alloc_.allocate(1);
  new (uv) edge_type(static_cast<graph_type&>(*this), u, v, val);
  ++this->edges_size_;
  return vertex_edge_iterator(static_cast<graph_type&>(*this), ukey, uv);
}


///-------------------------------------------------------------------------------------
/// undirected_adjacency_list
///

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const allocator_type& alloc)
      : base_type(alloc) {}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <typename GV_>
  requires (!std::is_void_v<GV_> && !std::is_same_v<std::remove_cvref_t<GV_>, undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>)
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const GV_& val,
                                                                                          const allocator_type&   alloc)
      : base_type(alloc)
      , graph_value_(val) {}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
template <typename GV_>
  requires (!std::is_void_v<GV_> && !std::is_same_v<std::remove_cvref_t<GV_>, undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>>)
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(GV_&&    val,
                                                                                          const allocator_type& alloc)
      : base_type(alloc)
      , graph_value_(std::forward<GV_>(val)) {}



// clang-format off
template <typename VV, typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
template <typename ERng, typename VRng, typename EProj, typename VProj, typename GV_>
  requires ranges::forward_range<ERng> 
        && ranges::input_range<VRng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
        && (!std::is_void_v<GV_>)
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const ERng&  erng,
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
template <typename VV, typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
template <typename ERng, typename EProj, typename GV_>
  requires ranges::forward_range<ERng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && (!std::is_void_v<GV_>)
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const ERng&  erng, 
                                                                              const EProj& eproj, 
                                                                              const GV_&   gv, 
                                                                              const Alloc& alloc)
      : undirected_adjacency_list(erng, vector<int>(), eproj, [](auto) 
{ return copyable_vertex_t<VId, VV>{VId()}; }, gv, alloc)
// clang-format on
{}

// Overload for GV=void: edge+vertex range constructor without graph value
// clang-format off
template <typename VV, typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
template <typename ERng, typename VRng, typename EProj, typename VProj>
  requires ranges::forward_range<ERng>
        && ranges::input_range<VRng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && std::regular_invocable<VProj, ranges::range_reference_t<VRng>>
        && std::is_void_v<GV>
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const ERng&  erng,
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

  // Evaluate max vertex key needed
  vertex_key_type max_vtx_key = vrng.empty() ? vertex_key_type(0) 
                                             : static_cast<vertex_key_type>(vrng.size() - 1);
  for (auto& e : erng) {
    auto&& edge_info = eproj(e);  // copyable_edge_t<VId, EV>
    max_vtx_key = max(max_vtx_key, max(edge_info.source_id, edge_info.target_id));
  }

  // add vertices
  this->vertices_.reserve(max_vtx_key + 1);
  if constexpr (!std::is_void_v<VV>) {
    for (auto& vtx : vrng) {
      auto&& [id, value] = vproj(vtx);  // copyable_vertex_t<VId, VV>
      create_vertex(value);
    }
  }
  this->vertices_.resize(max_vtx_key + 1); // assure expected vertices exist

  // add edges
  if (!ranges::empty(erng)) {
    auto&& first_edge_info = eproj(*ranges::begin(erng)); // first edge
    vertex_key_type tkey = first_edge_info.source_id;     // last in-vertex key
    for (auto& edge_data : erng) {
      auto&& edge_info = eproj(edge_data);  // copyable_edge_t<VId, EV>
      if (edge_info.source_id < tkey)
        this->throw_unordered_edges();

      vertex_edge_iterator uv;
      if constexpr (std::is_void_v<EV>) {
        uv = create_edge(edge_info.source_id, edge_info.target_id);
      } else {
        uv = create_edge(edge_info.source_id, edge_info.target_id, edge_info.value);
      }
      tkey = edge_info.source_id;
    }
  }
}

// Overload for GV=void: edge-only range constructor without graph value
// clang-format off
template <typename VV, typename EV, typename GV, integral VId, template <typename V, typename A> class VContainer, typename Alloc>
template <typename ERng, typename EProj>
  requires ranges::forward_range<ERng>
        && std::regular_invocable<EProj, ranges::range_reference_t<ERng>>
        && std::is_void_v<GV>
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(const ERng&  erng, 
                                                                              const EProj& eproj, 
                                                                              const Alloc& alloc)
      : undirected_adjacency_list(erng, vector<int>(), eproj, [](auto) 
{ return copyable_vertex_t<VId, VV>{VId()}; }, alloc)
// clang-format on
{}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(
      const initializer_list<tuple<vertex_key_type, vertex_key_type, edge_value_type>>& ilist, const Alloc& alloc)
      : base_type(ilist, alloc) {}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(
      const initializer_list<tuple<vertex_key_type, vertex_key_type>>& ilist, const Alloc& alloc)
      : base_type(ilist, alloc) {}

// Copy constructor - deep copies all vertices and edges
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::undirected_adjacency_list(
      const undirected_adjacency_list& other)
      : base_type(other)
      , graph_value_(other.graph_value_) {}

// Copy assignment operator - uses copy-and-swap idiom
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>&
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::operator=(
      const undirected_adjacency_list& other) {
  if (this != &other) {
    undirected_adjacency_list tmp(other);
    swap(tmp);
  }
  return *this;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::~undirected_adjacency_list() = default;

//-------------------------------------------------------------------------------------
// Vertex/edge modification methods (non-accessor)
//-------------------------------------------------------------------------------------

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::reserve_vertices(vertex_size_type n) {
  this->vertices_.reserve(n);
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::resize_vertices(vertex_size_type n) {
  this->vertices_.resize(n);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::resize_vertices(vertex_size_type         n,
                                                                                     const vertex_value_type& val) {
  this->vertices_.resize(n, val);
}

// Edge removal and graph operations
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::edge_iterator
undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::erase_edge(edge_iterator pos) {
  edge_type* uv = &*pos;
  ++pos;
  uv->~edge_type(); // unlinks from vertices
  this->edge_alloc_.deallocate(uv, 1);
  --this->edges_size_;
  return pos;
}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::clear() {
  // make sure edges are deallocated from this->edge_alloc_
  for (vertex_type& u : this->vertices_)
    u.clear_edges(*this);
  this->vertices_.clear(); // now we can clear the vertices
}


template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, GV, VId, VContainer, Alloc>::swap(undirected_adjacency_list& rhs) {
  using std::swap;
  swap(graph_value_, rhs.graph_value_);
  this->vertices_.swap(rhs.vertices_);
  swap(this->edges_size_, rhs.edges_size_);
  swap(this->edge_alloc_, rhs.edge_alloc_);
}

///-------------------------------------------------------------------------------------
/// ual_const_vertex_vertex_iterator
///
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::ual_const_vertex_vertex_iterator(
      vertex_edge_iterator const& uv)
      : uv_(uv) {}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::graph_type&
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::graph() noexcept {
  return uv_.graph();
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr const typename ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::graph_type&
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::graph() const noexcept {
  return uv_.graph();
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::const_vertex_iterator
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::other_vertex() const {
  return uv_->other_vertex(uv_.graph(), uv_.source_key());
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::vertex_key_type
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::other_vertex_key() const {
  return uv_->other_vertex_key(uv_.graph(), uv_.source_key());
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::reference
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator*() const noexcept {
  return *uv_->other_vertex(uv_.graph(), uv_.source_key());
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::pointer
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator->() const noexcept {
  return &**this;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>&
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator++() noexcept {
  ++uv_;
  return *this;
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator++(int) noexcept {
  ual_const_vertex_vertex_iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>&
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator--() noexcept {
  --uv_;
  return *this;
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>
ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator--(int) noexcept {
  ual_const_vertex_vertex_iterator tmp(*this);
  --*this;
  return tmp;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr bool ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator==(
      const ual_const_vertex_vertex_iterator& rhs) const noexcept {
  return uv_ == rhs.uv_;
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr bool ual_const_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator!=(
      const ual_const_vertex_vertex_iterator& rhs) const noexcept {
  return !operator==(rhs);
}


///-------------------------------------------------------------------------------------
/// ual_vertex_vertex_iterator
///
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::ual_vertex_vertex_iterator(
      vertex_edge_iterator const& uv)
      : base_t(uv) {}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::vertex_iterator
ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::other_vertex() {
  return uv_->other_vertex(uv_.graph(), uv_.source_key());
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::const_reference
ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator*() const {
  return *uv_->other_vertex(uv_.graph(), uv_.source_key());
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr typename ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::const_pointer
ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator->() const {
  return &**this;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>&
ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator++() {
  ++uv_;
  return *this;
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>
ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator++(int) {
  ual_vertex_vertex_iterator tmp(*this);
  ++*this;
  return tmp;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>&
ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator--() noexcept {
  --uv_;
  return *this;
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>
ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator--(int) noexcept {
  ual_vertex_vertex_iterator tmp(*this);
  --*this;
  return tmp;
}

template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr bool ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator==(
      const ual_vertex_vertex_iterator& rhs) const noexcept {
  return base_t::operator==(rhs);
}
template <typename VV,
          typename EV,
          typename GV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
constexpr bool ual_vertex_vertex_iterator<VV, EV, GV, VId, VContainer, Alloc>::operator!=(
      const ual_vertex_vertex_iterator& rhs) const noexcept {
  return base_t::operator!=(rhs);
}

///-------------------------------------------------------------------------------------
/// Implementations for undirected_adjacency_list<VV, EV, void, ...> specialization
/// (when GV=void, no graph_value_ member)
///

// Default allocator constructor
template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::undirected_adjacency_list(const allocator_type& alloc)
      : base_type(alloc) {}

// Copy constructor
template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::undirected_adjacency_list(
      const undirected_adjacency_list& other)
      : base_type(other) {}

// Destructor
template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::~undirected_adjacency_list() = default;

//-------------------------------------------------------------------------------------
// Vertex/edge modification methods (non-accessor) - GV=void specialization
//-------------------------------------------------------------------------------------

template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::reserve_vertices(vertex_size_type n) {
  this->vertices_.reserve(n);
}

template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::resize_vertices(vertex_size_type n) {
  this->vertices_.resize(n);
}

template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::resize_vertices(vertex_size_type n, 
                                                                                       const vertex_value_type& val) {
  this->vertices_.resize(n, val);
}

// Edge removal and graph operations - GV=void specialization
template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
typename undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::edge_iterator
undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::erase_edge(edge_iterator pos) {
  using edge_type = typename undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::edge_type;
  edge_type* uv = &*pos;
  ++pos;
  uv->~edge_type(); // unlinks from vertices
  this->edge_alloc_.deallocate(uv, 1);
  --this->edges_size_;
  return pos;
}

template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::clear() {
  // make sure edges are deallocated from this->edge_alloc_
  for (vertex_type& u : this->vertices_)
    u.clear_edges(*this);
  this->vertices_.clear(); // now we can clear the vertices
}

template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
void undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::swap(undirected_adjacency_list& rhs) {
  using std::swap;
  this->vertices_.swap(rhs.vertices_);
  swap(this->edges_size_, rhs.edges_size_);
  swap(this->edge_alloc_, rhs.edge_alloc_);
  // Note: No graph_value_ in the GV=void specialization
}

template <typename VV,
          typename EV,
          integral VId,
          template <typename V, typename A>
          class VContainer,
          typename Alloc>
undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>&
undirected_adjacency_list<VV, EV, void, VId, VContainer, Alloc>::operator=(
      const undirected_adjacency_list& other) {
  if (this != &other) {
    undirected_adjacency_list tmp(other);
    swap(tmp);
  }
  return *this;
}

} // namespace graph::container

#endif // UNDIRECTED_ADJ_LIST_IMPL_HPP
