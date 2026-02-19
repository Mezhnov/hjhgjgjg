package com.example.playstoreclone.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.bumptech.glide.load.resource.bitmap.RoundedCorners
import com.example.playstoreclone.R
import com.example.playstoreclone.databinding.ItemAppListBinding
import com.example.playstoreclone.models.AppModel

class AppListAdapter(
    private val apps: List<AppModel>,
    private val onAppClick: (AppModel) -> Unit
) : RecyclerView.Adapter<AppListAdapter.ViewHolder>() {

    inner class ViewHolder(private val binding: ItemAppListBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(app: AppModel, position: Int) {
            binding.apply {
                tvPosition.text = (position + 1).toString()
                tvAppName.text = app.name
                tvCategory.text = app.category
                tvRating.text = app.rating.toString()
                tvSize.text = app.size

                Glide.with(itemView.context)
                    .load(app.iconUrl)
                    .placeholder(R.drawable.ic_placeholder)
                    .transform(RoundedCorners(28))
                    .into(ivAppIcon)

                root.setOnClickListener { onAppClick(app) }
            }
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = ItemAppListBinding.inflate(
            LayoutInflater.from(parent.context), parent, false
        )
        return ViewHolder(binding)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(apps[position], position)
    }

    override fun getItemCount() = apps.size
}
